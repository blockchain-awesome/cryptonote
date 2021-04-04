
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>
#include "transaction.h"
#include "./crypto.h"

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

namespace stream
{
  namespace cryptonote
  {

    struct input_visitor_t : boost::static_visitor<>
    {
      input_visitor_t(Writer &o) : o(o) {}

      template <typename T>
      void operator()(T &param) { o << param; }

      Writer &o;
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

    Writer &operator<<(Writer &o, key_input_t &v)
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

    Writer &operator<<(Writer &o, multi_signature_input_t &v)
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

    Writer &operator<<(Writer &o, transaction_input_t &v)
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

    Writer &operator<<(Writer &o, key_output_t &v)
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

    Writer &operator<<(Writer &o, multi_signature_output_t &v)
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

    Writer &operator<<(Writer &o, transaction_output_target_t &v)
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
    Writer &operator<<(Writer &o, transaction_output_t &v)
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
    Writer &operator<<(Writer &o, transaction_prefix_t &v)
    {
      o << v.version;
      o << v.unlockTime;
      o << v.inputs;
      o << v.outputs;
      o << v.extra;
      return o;
    }

    // Reader &operator>>(Reader &i, transaction_t &v)
    // {
    //   i >> (*(transaction_prefix_t *)&v);
    //   size_t size = v.inputs.size();
    //   v.signatures.resize(size);
    //   bool empty = v.signatures.empty();

    //   i >> v.signatures;
    // }

  }
}