
#include "multisignature_output_usage.h"

namespace cryptonote
{

  void serialize(multisignature_output_usage_t &usage, ISerializer &s)
  {
    s(usage.transactionIndex, "txindex");
    s(usage.outputIndex, "outindex");
    s(usage.isUsed, "used");
  }
};