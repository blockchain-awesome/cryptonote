#include "BinarySerializer.h"
#include "BinaryInputStreamSerializer.h"
#include "BinaryOutputStreamSerializer.h"
#include "stream/reader.h"
#include "stream/reader.h"
#include "stream/writer.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include "common/file.h"

namespace cryptonote
{
BinarySerializer::BinarySerializer(const std::string &filename) : m_filename(filename)
{
}
bool BinarySerializer::store(bool create)
{
  try
  {
    std::ifstream f(m_filename.c_str());
    if (!f.good())
    {
      std::fstream fs = std::file::open(m_filename);
      fs.close();
    }
    std::ofstream dataFile;

    dataFile.open(m_filename, std::ios_base::binary | std::ios_base::out | std::ios::trunc);
    if (dataFile.fail())
    {
      return false;
    }

    Writer stream(dataFile);
    BinaryOutputStreamSerializer out(stream);
    serialize(out);

    if (dataFile.fail())
    {
      return false;
    }

    dataFile.flush();
  }
  catch (std::exception &)
  {
    return false;
  }

  return true;
}
bool BinarySerializer::load()
{
  try
  {
    std::ifstream dataFile;
    dataFile.open(m_filename, std::ios_base::binary | std::ios_base::in);
    if (dataFile.fail())
    {
      std::cout << "Fail to open binary file: " << m_filename << std::endl;
      std::cout << "Error reason: " << strerror(errno) << std::endl;
      return false;
    }

    Reader stream(dataFile);
    BinaryInputStreamSerializer in(stream);
    serialize(in);
    return !dataFile.fail();
  }
  catch (std::exception &e)
  {
    std::cout << "Load exception :" << e.what() << std::endl;
    return false;
  }
}
binary_array_t BinarySerializer::to()
{
  binary_array_t result;
  std::ostringstream oss;
  Writer stream(oss);
  BinaryOutputStreamSerializer ba(stream);
  serialize(ba);
  result.resize(oss.str().length());
  result.assign(oss.str().begin(), oss.str().end());
  return result;
}
bool BinarySerializer::from(const binary_array_t &blob)
{
  // Reader stream(blob.data(), blob.size());
  const unsigned char * b = static_cast<const unsigned char *>(blob.data());
  membuf mem((char *)(b), (char *)(b + blob.size()));
  std::istream istream(&mem);
  Reader stream(istream);
  BinaryInputStreamSerializer ba(stream);
  return serialize(ba);
}

} // namespace cryptonote
