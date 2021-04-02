
#include "writer.h"

Writer::Writer(std::ostream &out) : out(out)
{
}

size_t Writer::writeSome(const void *data, size_t size)
{
  out.write(static_cast<const char *>(data), size);
  if (out.bad())
  {
    return 0;
  }

  return size;
}

void Writer::write(const void *data, size_t size)
{
  {
    while (size > 0)
    {
      size_t writtenSize = writeSome(data, size);
      if (writtenSize == 0)
      {
        throw std::runtime_error("Failed to write to Writer");
      }

      data = static_cast<const uint8_t *>(data) + writtenSize;
      size -= writtenSize;
    }
  }
}

void Writer::write(std::vector<uint8_t> &data, size_t size)
{
  data.resize(size);
  write(data.data(), size);
}

void Writer::write(std::string &data, size_t size)
{
  std::vector<char> temp(size);
  write(temp.data(), size);
  data.assign(temp.data(), size);
}