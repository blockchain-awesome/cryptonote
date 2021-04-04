
#include "transaction.h"
namespace stream
{
  namespace transaction
  {

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
  }
}