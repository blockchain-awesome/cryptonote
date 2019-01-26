#pragma once
#include <string>
#include <sstream>

namespace std
{

namespace str
{
template <typename T>
std::string from(T i)
{
    std::ostringstream oss;
    oss << i;
    return oss.str();
}
} // namespace str
} // namespace std