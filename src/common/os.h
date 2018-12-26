#pragma once

#include <string>

namespace os
{
namespace version
{
std::string get();
}
namespace appdata
{
std::string path();
}

extern const std::string getCoinFile(const std::string &filename);

} // namespace os