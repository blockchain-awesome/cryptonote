#include "block_index.h"

using namespace serialize;

bool BlockIndexStream::serialize(std::ostream &o)
{


    const auto &begin = m_container.begin();
    const auto &end = m_container.end();
    size_t size = m_container.size();
    o << size;
    
    auto & i = (auto &)begin;
    for ( ; i != end; i++)
    {
        o << (hash_t)(*i);
    }
    return true;
}
bool BlockIndexStream::serialize(std::istream &i)
{
}
