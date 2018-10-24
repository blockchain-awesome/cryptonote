#include "filesystem.h"
#include <algorithm>

namespace std
{
namespace filesystem
{
namespace path
{
const char GENERIC_PATH_SEPARATOR = '/';

#ifdef _WIN32
const char NATIVE_PATH_SEPARATOR = '\\';
#else
const char NATIVE_PATH_SEPARATOR = '/';
#endif
std::string generic_string(const std::string &nativePath)
{
    if (GENERIC_PATH_SEPARATOR == NATIVE_PATH_SEPARATOR)
    {
        return nativePath;
    }
    std::string genericPath(nativePath);
    std::replace(genericPath.begin(), genericPath.end(), NATIVE_PATH_SEPARATOR, GENERIC_PATH_SEPARATOR);
    return genericPath;
}

} // namespace path
} // namespace filesystem
} // namespace std