

#include <vector>
#include <map>
#include "transaction_index.h"


namespace cryptonote
{

bool serialize(std::vector<std::pair<TransactionIndex, uint16_t>> &value, Common::StringView name, cryptonote::ISerializer &s)
{
    const size_t elementSize = sizeof(std::pair<TransactionIndex, uint16_t>);
    size_t size = value.size() * elementSize;

    if (!s.beginArray(size, name))
    {
        return false;
    }

    if (s.type() == cryptonote::ISerializer::INPUT)
    {
        if (size % elementSize != 0)
        {
            throw std::runtime_error("Invalid vector size");
        }
        value.resize(size / elementSize);
    }

    if (size)
    {
        s.binary(value.data(), size, "");
    }

    s.endArray();
    return true;
}

void serialize(TransactionIndex &value, ISerializer &s)
{
    s(value.block, "block");
    s(value.transaction, "tx");
}

} // namespace cryptonote