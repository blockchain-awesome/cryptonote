
#include "reader.h"

struct membuf : std::streambuf
{
  membuf(char *begin, char *end)
  {
    this->setg(begin, begin, end);
  }
};

Reader::Reader(const char *buffer, size_t size)
{
    membuf sbuf((char *)buffer, (char *)buffer + size);
    std::istream in(&sbuf);
    this->in = &in;
}

Reader::Reader(std::istream *in) : in(in)
{
}

size_t Reader::readSome(void *data, size_t size)
{
  in->read(static_cast<char *>(data), size);
  return in->gcount();
}

void Reader::read(void *data, size_t size)
{
  while (size > 0)
  {
    size_t readSize = readSome(data, size);
    if (readSize == 0)
    {
      throw std::runtime_error("Failed to read from IInputStream");
    }

    data = static_cast<uint8_t *>(data) + readSize;
    size -= readSize;
  }
}

void Reader::read(std::vector<uint8_t> &data, size_t size)
{
  data.resize(size);
  read(data.data(), size);
}

void Reader::read(std::string &data, size_t size)
{
  std::vector<char> temp(size);
  read(temp.data(), size);
  data.assign(temp.data(), size);
}