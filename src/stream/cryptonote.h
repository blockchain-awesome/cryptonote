
#pragma once

#include "crypto/types.h"
#include "reader.h"
#include "writer.h"

namespace stream
{
  namespace cryptonote
  {
    Reader &operator>>(Reader &i, public_key_t &v);
    template <typename T>
    Reader &operator>>(Reader &i, std::vector<T> &v)
    {
      size_t size = 0;
      i >> size;

      v.resize(size);
      for (auto &item : v)
      {
        i << item;
      }

      return i;
    }

    Writer &operator<<(Writer &o, const public_key_t &v);

    template <typename T>
    Writer &operator<<(Writer &o, std::vector<T> &v)
    {
      size_t size = v.size();
      o << size;
      for (auto &item : v)
      {
        o << item;
      }

      return o;
    }

  }
}
