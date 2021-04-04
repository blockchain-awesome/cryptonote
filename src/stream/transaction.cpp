
#include "transaction.h"

namespace stream
{
  namespace transaction
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

    // Reader &operator>>(Reader &i, transaction_prefix_t &v)
    // {
    //   i >> v.version;
    //   i >> v.unlockTime;
    //   i >> v.inputs;
    //   i >> v.outputs;
    // }

    //     void serialize(transaction_prefix_t &txP, ISerializer &serializer)
    // {
    //   serializer(txP.version, "version");

    //   // if (CURRENT_TRANSACTION_VERSION < txP.version) {
    //   //   throw std::runtime_error("Wrong transaction version");
    //   // }

    //   serializer(txP.unlockTime, "unlock_time");
    //   serializer(txP.inputs, "vin");
    //   serializer(txP.outputs, "vout");
    //   serializeAsBinary(txP.extra, "extra", serializer);
    // }

    //     void serialize(base_input_t &gen, ISerializer &serializer)
    // {
    //   serializer(gen.blockIndex, "height");
    // }

    // void serialize(key_input_t &key, ISerializer &serializer)
    // {
    //   serializer(key.amount, "amount");
    //   serializeVarintVector(key.outputIndexes, serializer, "key_offsets");
    //   serializer(key.keyImage, "k_image");
    // }

    // void serialize(multi_signature_input_t &multisignature, ISerializer &serializer)
    // {
    //   serializer(multisignature.amount, "amount");
    //   serializer(multisignature.signatureCount, "signatures");
    //   serializer(multisignature.outputIndex, "outputIndex");
    // }
  }
}