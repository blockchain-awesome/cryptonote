#include "file.h"
#include <iostream>
#include <cstdio>

namespace std
{
namespace file
{

bool exists(string filename)
{
  ifstream f(filename.c_str());
  return f.good();
}

bool unlink(string filename)
{
  if (!exists(filename))
  {
    return true;
  }
  if (remove(filename.c_str()) != 0)
  {
    return false;
  }
  return true;
}

fstream open(const string filename, bool forceCreate)
{
  fstream fs;
  fs.open(filename, fstream::binary | fstream::in | fstream::out);
  if (!fs)
  {
    if (!forceCreate)
    {
      std::cout << "fail to open and return" << std::endl;

      return fs;
    }
    std::cout << "fail to open and create" << std::endl;

    fs.open(filename, fstream::binary | fstream::trunc | fstream::out);
    fs.close();
    // re-open with original flags
    fs.open(filename, fstream::binary | fstream::in | fstream::out);
  }
  return fs;
}
bool write(const string filename, const char *data, size_t size, size_t offset, bool forceCreate)
{
  fstream fs = open(filename, forceCreate);
  if (!fs)
  {
    return false;
  }
  fs.seekp(offset);
  return !!fs.write(data, size);
}

bool read(const string filename, char *data, size_t size, bool forceCreate)
{
  fstream fs = open(filename, forceCreate);
  if (!fs)
  {
    return false;
  }
  fs.read(data, size);
  return !!fs;
}
} // namespace file
} // namespace std