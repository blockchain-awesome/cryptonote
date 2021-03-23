
#include <sstream>
#include <fstream>
#include <cstring>
#include "binary.h"

IBinary::IBinary(){};
std::string IBinary::to(const void *data, size_t size)
{
  return std::string(static_cast<const char *>(data), size);
}
std::string IBinary::to(const binary_array_t &data)
{
  return std::string(reinterpret_cast<const char *>(data.data()), data.size());
}

binary_array_t IBinary::to()
{
  binary_array_t binaryArray;
  std::ostringstream oss;
  this->write(oss);
  binaryArray.resize(oss.str().length());
  memcpy(&binaryArray[0], oss.str().c_str(), oss.str().length());
  return binaryArray;
}

binary_array_t IBinary::from(const std::string &data)
{
  auto dataPtr = reinterpret_cast<const uint8_t *>(data.data());
  return binary_array_t(dataPtr, dataPtr + data.size());
}

void IBinary::from(const binary_array_t &blob)
{
  std::istringstream iss(std::string(blob.begin(), blob.end()));
  this->read(iss);
}

bool IBinary::save(const std::string &filename)
{
  try
  {
    std::ofstream ofs;
    ofs.open(filename, std::ios_base::binary | std::ios_base::out | std::ios::trunc);
    if (ofs.fail())
    {
      std::cout << "Fail to save to file: " << filename << "!" << std::endl;
      std::cout << "Error Reason: " << strerror(errno) << "!" << std::endl;
      return false;
      return false;
    }

    this->write(ofs);

    if (ofs.fail())
    {
      return false;
    }

    ofs.flush();
  }
  catch (std::exception &)
  {
    return false;
  }

  return true;
}

bool IBinary::load(const std::string &filename)
{
  try
  {
    std::ifstream ifs;
    ifs.open(filename, std::ios_base::binary | std::ios_base::in);
    if (ifs.fail())
    {
      std::cout << "Fail to open a binary file: " << filename << "!" << std::endl;
      std::cout << "Error Reason: " << strerror(errno) << "!" << std::endl;
      return false;
    }
    this->read(ifs);
    return !ifs.fail();
  }
  catch (std::exception &e)
  {
    std::cout << "Load exception :" << e.what() << std::endl;
    return false;
  }
}

std::istream &operator>>(std::istream &is, IBinary &base)
{
  base.read(is);
  return is;
}

std::ostream &operator<<(std::ostream &os, const IBinary &base)
{
  base.write(os);
  return os;
}

namespace binary
{

  bool load(const std::string &filepath, std::string &buf)
  {
    try
    {
      std::ifstream fstream;
      fstream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
      fstream.open(filepath, std::ios_base::binary | std::ios_base::in | std::ios::ate);

      size_t fileSize = static_cast<size_t>(fstream.tellg());
      buf.resize(fileSize);

      if (fileSize > 0)
      {
        fstream.seekg(0, std::ios::beg);
        fstream.read(&buf[0], buf.size());
      }
    }
    catch (const std::exception &)
    {
      return false;
    }

    return true;
  }

  bool save(const std::string &filepath, const std::string &buf)
  {
    try
    {
      std::ofstream fstream;
      fstream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
      fstream.open(filepath, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
      fstream << buf;
    }
    catch (const std::exception &)
    {
      return false;
    }

    return true;
  }
}