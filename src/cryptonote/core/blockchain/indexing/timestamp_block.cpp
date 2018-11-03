#include "timestamp_block.h"

#include "cryptonote/core/CryptoNoteTools.h"

namespace cryptonote
{

bool TimestampBlocksIndex::add(uint64_t timestamp, const crypto::hash_t &hash)
{
    index.emplace(timestamp, hash);
    return true;
}

bool TimestampBlocksIndex::remove(uint64_t timestamp, const crypto::hash_t &hash)
{
    auto range = index.equal_range(timestamp);
    for (auto iter = range.first; iter != range.second; ++iter)
    {
        if (iter->second == hash)
        {
            index.erase(iter);
            return true;
        }
    }

    return false;
}

bool TimestampBlocksIndex::find(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t hashesNumberLimit, std::vector<crypto::hash_t> &hashes, uint32_t &hashesNumberWithinTimestamps)
{
    uint32_t hashesNumber = 0;
    if (timestampBegin > timestampEnd)
    {
        //std::swap(timestampBegin, timestampEnd);
        return false;
    }
    auto begin = index.lower_bound(timestampBegin);
    auto end = index.upper_bound(timestampEnd);

    hashesNumberWithinTimestamps = static_cast<uint32_t>(std::distance(begin, end));

    for (auto iter = begin; iter != end && hashesNumber < hashesNumberLimit; ++iter)
    {
        ++hashesNumber;
        hashes.emplace_back(iter->second);
    }
    return hashesNumber > 0;
}

void TimestampBlocksIndex::clear()
{
    index.clear();
}

void TimestampBlocksIndex::serialize(ISerializer &s)
{
    s(index, "index");
}
} // namespace cryptonote