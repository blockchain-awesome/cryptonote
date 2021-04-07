
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>
#include "transaction.h"
#include "crypto.h"
#include "cryptonote/crypto/crypto.h"

typedef enum : uint8_t
{
  BASE = 0xFF,
  KEY = 0x2,
  SIGN = 0x3,
  TRANSACTION = 0xcc,
  BLOCK = 0Xbb
} ioput_t;

struct tag_getter_t : boost::static_visitor<uint8_t>
{
  uint8_t operator()(const cryptonote::base_input_t) { return BASE; }
  uint8_t operator()(const cryptonote::key_input_t) { return KEY; }
  uint8_t operator()(const cryptonote::multi_signature_input_t) { return SIGN; }
  uint8_t operator()(const cryptonote::key_output_t) { return KEY; }
  uint8_t operator()(const cryptonote::multi_signature_output_t) { return SIGN; }
  uint8_t operator()(const cryptonote::transaction_t) { return TRANSACTION; }
};

struct input_visitor_t : boost::static_visitor<>
{
  input_visitor_t(Writer &o) : o(o) {}

  template <typename T>
  void operator()(T &param) { o << param; }

  Writer &o;
};

struct signature_size_visitor : public boost::static_visitor<size_t>
{
  size_t operator()(const base_input_t &txin) const { return 0; }
  size_t operator()(const key_input_t &txin) const { return txin.outputIndexes.size(); }
  size_t operator()(const multi_signature_input_t &txin) const { return txin.signatureCount; }
};

Reader &operator>>(Reader &i, base_input_t &v)
{
  i >> v.blockIndex;
  return i;
}
Writer &operator<<(Writer &o, const base_input_t &v)
{
  o << v.blockIndex;
  return o;
}

Reader &operator>>(Reader &i, key_input_t &v)
{
  i >> v.amount;
  i >> v.outputIndexes;
  i >> v.keyImage;
  return i;
}

Writer &operator<<(Writer &o, const key_input_t &v)
{
  o << v.amount;
  o << v.outputIndexes;
  o << v.keyImage;
  return o;
}

Reader &operator>>(Reader &i, multi_signature_input_t &v)
{
  i >> v.amount;
  i >> v.signatureCount;
  i >> v.outputIndex;
  return i;
}

Writer &operator<<(Writer &o, const multi_signature_input_t &v)
{
  o << v.amount;
  o << v.signatureCount;
  o << v.outputIndex;
  return o;
}

Reader &operator>>(Reader &i, transaction_input_t &v)
{
  uint8_t tag;
  i.read(&tag, 1);

  switch (tag)
  {
  case BASE:
  {
    base_input_t key;
    i >> key;
    v = key;
    break;
  }
  case KEY:
  {
    key_input_t key;
    i >> key;
    v = key;
    break;
  }
  case SIGN:
  {
    multi_signature_input_t key;
    i >> key;
    v = key;
    break;
  }
  default:
    throw std::runtime_error("Unknown variant tag");
  }
  return i;
}

Writer &operator<<(Writer &o, const transaction_input_t &v)
{
  tag_getter_t getter;
  uint8_t tag = boost::apply_visitor(getter, v);

  o << tag;

  input_visitor_t visitor(o);
  boost::apply_visitor(visitor, v);
  return o;
}

Reader &operator>>(Reader &i, key_output_t &v)
{
  i >> v.key;
  return i;
}

Writer &operator<<(Writer &o, const key_output_t &v)
{
  o << v.key;
  return o;
}

Reader &operator>>(Reader &i, multi_signature_output_t &v)
{
  i >> v.keys;
  i >> v.requiredSignatureCount;
  return i;
}

Writer &operator<<(Writer &o, const multi_signature_output_t &v)
{
  o << v.keys;
  o << v.requiredSignatureCount;
  return o;
}
Reader &operator>>(Reader &i, transaction_output_target_t &v)
{
  uint8_t tag;
  i.read(&tag, 1);
  switch (tag)
  {

  case KEY:
  {
    key_output_t key;
    i >> key;
    v = key;
    break;
  }
  case SIGN:
  {
    multi_signature_output_t key;
    i >> key;
    v = key;
    break;
  }
  default:
    throw std::runtime_error("Unknown variant tag");
  }
  return i;
}

Writer &operator<<(Writer &o, const transaction_output_target_t &v)
{
  tag_getter_t getter;
  uint8_t tag = boost::apply_visitor(getter, v);

  o << tag;

  input_visitor_t visitor(o);
  boost::apply_visitor(visitor, v);
  return o;
}

Reader &operator>>(Reader &i, transaction_output_t &v)
{
  i >> v.amount;
  i >> v.target;
  return i;
}
Writer &operator<<(Writer &o, const transaction_output_t &v)
{
  o << v.amount;
  o << v.target;
  return o;
}

Reader &operator>>(Reader &i, transaction_prefix_t &v)
{
  i >> v.version;
  i >> v.unlockTime;
  i >> v.inputs;
  i >> v.outputs;
  i >> v.extra;
  return i;
}
Writer &operator<<(Writer &o, const transaction_prefix_t &v)
{
  o << v.version;
  o << v.unlockTime;
  o << v.inputs;
  o << v.outputs;
  o << v.extra;
  return o;
}

Reader &operator>>(Reader &i, transaction_t &v)
{
  i >> (*(transaction_prefix_t *)&v);
  size_t size = v.inputs.size();
  v.signatures.resize(size);
  bool empty = v.signatures.empty();
  bool equalSize = v.inputs.size() == v.signatures.size();
  if (!empty && !equalSize)
  {
    throw std::runtime_error("Serialization error: unexpected signatures size");
  }
  for (size_t j = 0; j < size; j++)
  {
    signature_size_visitor visitor;
    size_t ssize = boost::apply_visitor(visitor, v.inputs[j]);
    if (empty)
    {
      if (ssize == 0)
      {
        continue;
      }
      else
      {
        throw std::runtime_error("Serialization error: signatures are not expected");
      }
    }
    std::vector<signature_t> signatures(ssize);
    for (signature_t &sig : signatures)
    {
      i >> sig;
    }
    v.signatures[j] = std::move(signatures);
  }
  return i;
}

Writer &operator<<(Writer &o, const transaction_t &v)
{
  o << (*(transaction_prefix_t *)&v);

  size_t size = v.inputs.size();
  bool empty = v.signatures.empty();
  bool equalSize = v.inputs.size() == v.signatures.size();
  if (!empty && !equalSize)
  {
    throw std::runtime_error("Serialization error: unexpected signatures size");
  }

  for (size_t j = 0; j < size; j++)
  {
    signature_size_visitor visitor;
    size_t ssize = boost::apply_visitor(visitor, v.inputs[j]);
    if (empty)
    {
      if (ssize == 0)
      {
        continue;
      }
      else
      {
        throw std::runtime_error("Serialization error: signatures are not expected");
      }
    }
    for (const signature_t sig : v.signatures[j])
    {
      o << sig;
    }
  }
  return o;
}

Reader &operator>>(Reader &i, transaction_index_t &v)
{
  i >> v.block;
  i >> v.transaction;
  return i;
}

Writer &operator<<(Writer &o, const transaction_index_t &v)
{
  o << v.block;
  o << v.transaction;
  return o;
}

// Reader &operator>>(Reader &i, transaction_map_t &v)
// {

//   size_t size = 0;

//   i >> size;

//   for (size_t j = 0; j < size; j++)
//   {
//     hash_t key;
//     transaction_index_t idx;
//     i >> key;
//     i >> idx;
//     v.insert(std::make_pair<hash_t, transaction_index_t>(std::move(key), std::move(idx)));
//   }
//   return i;
// }

// Writer &operator<<(Writer &o, const transaction_map_t &v)
// {

//   size_t size = v.size();
//   o << size;

//   for (auto &kv : v)
//   {
//     o << kv.first;
//     o << kv.second;
//   }
//   return o;
// }

Reader &operator>>(Reader &i, multisignature_output_usage_t &v)
{
  i >> v.transactionIndex;
  i >> v.outputIndex;
  i >> v.isUsed;
  return i;
}

Writer &operator<<(Writer &o, const multisignature_output_usage_t &v)
{
  o << v.transactionIndex;
  o << v.outputIndex;
  o << v.isUsed;
  return o;
}

Reader &operator>>(Reader &i, transaction_entry_t &v)
{
  i >> v.tx;
  i >> v.m_global_output_indexes;
  return i;
}

Writer &operator<<(Writer &o, const transaction_entry_t &v)
{
  o << v.tx;
  o << v.m_global_output_indexes;
  return o;
}

Reader &operator>>(Reader &i, transaction_details_t &v)
{
  i >> v.id;
  i >> v.blobSize;
  i >> v.fee;
  i >> v.tx;
  i >> v.maxUsedBlock.height;
  i >> v.maxUsedBlock.id;
  i >> v.lastFailedBlock.height;
  i >> v.lastFailedBlock.id;
  i >> v.keptByBlock;
  i >> reinterpret_cast<uint64_t &>(v.receiveTime);
  return i;
}

Writer &operator<<(Writer &o, const transaction_details_t &v)
{
  o << v.id;
  o << v.blobSize;
  o << v.fee;
  o << v.tx;
  o << v.maxUsedBlock.height;
  o << v.maxUsedBlock.id;
  o << v.lastFailedBlock.height;
  o << v.lastFailedBlock.id;
  o << v.keptByBlock;
  o << reinterpret_cast<const uint64_t &>(v.receiveTime);

  return o;
}
