
#include "complex.h"

namespace serialize
{

  namespace complex
  {
    std::istream &operator>>(std::istream &i, std::string &v)
    {
      size_t size = v.length();
      varint(i, size);
      bytes(i, (void *)&(*v.data()), size);
      return i;
    }

    std::ostream &operator<<(std::ostream &o, std::string &v)
    {
      size_t size;
      varint(o, size);
      if (size > 0)
      {
        std::vector<uint8_t> temp;
        temp.resize(size);
        bytes(o, &temp[0], size);
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