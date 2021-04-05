
#pragma once

#include "crypto/types.h"
#include "reader.h"
#include "writer.h"
#include <assert.h>

namespace stream
{
  namespace cryptonote
  {
    template <typename T>
    Reader &operator>>(Reader &i, std::vector<T> &v)
    {
      size_t size = 0;
      i >> size;

      assert(size >= 0);

      v.resize(size);
      if (size == 0) {
        v.clear();
      }
      for (auto &item : v)
      {
        i >> item;
      }

      return i;
    }

    template <typename T>
    Writer &operator<<(Writer &o, const std::vector<T> &v)
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
