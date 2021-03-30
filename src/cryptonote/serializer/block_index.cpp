#include "block_index.h"
#include "stream/crypto.h"

using namespace serialize;
using namespace serialize::crypto;

bool BlockIndexStream::serialize(std::ostream &o)
{

  const auto &begin = m_container.begin();
  const auto &end = m_container.end();
  size_t size = m_container.size();
  o << size;

  auto &pb = (ContainerT::nth_index<1>::type::iterator &)begin;
  auto &pe = (ContainerT::nth_index<1>::type::iterator &)end;

  ContainerT::nth_index<1>::type::iterator &i = pb;
  for (; i != pe; i++)
  {
    o << (hash_t)(*i);
  }
  return true;
}

bool BlockIndexStream::serialize(std::istream &i)
{

  size_t size = 0;

  i >> size;

  while (size--)
  {
    hash_t h;
    i >> h;
    m_container.push_back(h);
  }
}
