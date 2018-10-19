#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "types.h"
#include "i_reader.h"
#include "i_writer.h"

namespace cryptonote
{

class ITransaction : public ITransactionReader,
                     public ITransactionWriter
{
  public:
    virtual ~ITransaction() {}
};
} // namespace cryptonote