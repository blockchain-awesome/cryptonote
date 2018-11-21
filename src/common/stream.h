#pragma once

#include <string>
#include <sstream> 
#include <vector>

namespace stream
{
template <typename T>
T fromString(const std::string &text)
{ // Throws on error
  T value;
  std::istringstream ss(text);
  ss >> value;
  if (ss.fail())
  {
    throw std::runtime_error("fromString: unable to parse value");
  }

  return value;
}

template <typename T>
bool fromString(const std::string &text, T &value)
{ // Does not throw
  std::istringstream s(text);
  s >> value;
  return !s.fail();
}

template <typename T>
std::string toString(const T &value)
{ // Does not throw
  std::ostringstream s;
  s << value;
  return s.str();
}

template <typename T>
void toString(const T &value, std::string &text)
{ // Does not throw
  std::ostringstream s;
  s << value;
  text += s.str();
}
extern bool load(const std::string& filepath, std::string& buf);
extern bool save(const std::string& filepath, const std::string& buf);

} // namespace
