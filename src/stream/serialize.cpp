#include "serialize.h"

namespace stream
{

  vistream &bytes(vistream &i, char *v, const size_t len)
  {
    i.i.read(v, len);
    return i;
  }

  vostream &bytes(vostream &o, const char *v, const size_t len)
  {
    o.o.write(v, len);
    return o;
  }

}