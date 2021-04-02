
#include <vector>
#include "complex.h"

namespace stream
{

  namespace complex
  {
    vistream &operator>>(vistream &i, std::string &v)
    {
      size_t size = v.length();
      varint(i, size);
      bytes(i, (char *)&(*v.data()), size);
      return i;
    }

    vostream &operator<<(vostream &o, std::string &v)
    {
      size_t size;
      varint(o, size);
      if (size > 0)
      {
        std::vector<uint8_t> temp;
        temp.resize(size);
        bytes(o, (char *)&temp[0], size);
        v.reserve(size);
        v.assign((char *)temp.data(), size);
      }
      else
      {
        v.clear();
      }

      return o;
    }
  }

}