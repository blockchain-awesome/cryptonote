
#include "transaction.h"
#include "./crypto.h"

namespace stream
{
  namespace cryptonote
  {

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
      case 0xff:
      {
        base_input_t key;
        i >> key;
        v = key;
        break;
      }
      case 0x2:
      {
        key_input_t key;
        i >> key;
        v = key;
        break;
      }
      case 0x3:
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

  }
}