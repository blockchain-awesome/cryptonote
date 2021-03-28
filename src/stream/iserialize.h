#include <istream>
#include <ostream>
namespace serialize
{

  class Base
  {
    virtual bool serialize(std::ostream &o) const = 0;
    virtual bool serialize(std::istream &i) const = 0;
    friend std::ostream &operator>>(std::ostream &o, const Base &value);
    friend std::istream &operator<<(std::istream &o, const Base &value);
  };

  std::ostream &operator>>(std::ostream &o, const Base &value)
  {
    value.serialize(o);
    return o;
  };

  std::istream &operator<<(std::istream &i, const Base &value)
  {
    value.serialize(i);
    return i;
  };

}
