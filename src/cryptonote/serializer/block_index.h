#include "crypto/types.h"
#include "stream/primitive.h"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/random_access_index.hpp>

using namespace serialize;

class BlockIndexStream : public serialize::IStream
{
    BlockIndexStream() : m_index(m_container.get<1>()) {}

    bool serialize(std::ostream &o);
    bool serialize(std::istream &i);
    typedef boost::multi_index_container<
        hash_t,
        boost::multi_index::indexed_by<
            boost::multi_index::random_access<>,
            boost::multi_index::hashed_unique<boost::multi_index::identity<hash_t>>>>
        ContainerT;

    ContainerT m_container;
    ContainerT::nth_index<1>::type &m_index;

};