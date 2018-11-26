#include <fstream>
namespace std
{
namespace file
{
bool unlink(string filename);
fstream open(const string filename, bool forceCreate = false);
bool write(const string filename, const char *data, size_t size, size_t offset = 0, bool forceCreate = false);
bool read(const string filename, char *data, size_t size, bool forceCreate = false);
} // namespace file
} // namespace std