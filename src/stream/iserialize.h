#include <istream>
#include <ostream>

namespace serialize
{

  class Base
  {
    virtual bool serialize(std::ostream &o) = 0;
    virtual bool serialize(std::istream &i) = 0;
    friend std::ostream &operator>>(std::ostream &o, Base &value);
    friend std::istream &operator<<(std::istream &o, Base &value);
  };

  std::ostream &operator>>(std::ostream &o, Base &value)
  {
    value.serialize(o);
    return o;
  };

  std::istream &operator<<(std::istream &i, Base &value)
  {
    value.serialize(i);
    return i;
  };

}
