#pragma once
#include "common/binary_array.h"
#include "ISerializer.h"

namespace cryptonote
{

class BinarySerializer
{
  public:
    BinarySerializer(const std::string &filename);
    bool store(bool create = true);
    bool load();
    binary_array_t to();
    bool from(const binary_array_t &blob);
    virtual bool serialize(cryptonote::ISerializer &serializer) = 0;
  protected:
    const std::string m_filename;
};
} // namespace cryptonote
