#include <cryptonote.h>
#include "stream/reader.h"
#include "stream/writer.h"

#include <fstream>

namespace stream
{

  namespace cryptonote
  {

    template <typename T>

    binary_array_t serialize(const T &t)
    {
      binary_array_t binaryArray;
      std::ostringstream oss;
      Writer stream(oss);
      stream << t;
      binaryArray.resize(oss.str().length());
      memcpy(&binaryArray[0], oss.str().c_str(), oss.str().length());
      return binaryArray;
    }
    template <typename T>
    void unserialize(T &t, const binary_array_t &v)
    {
      const unsigned char *b = static_cast<const unsigned char *>(v.data());
      membuf mem((char *)(b), (char *)(b + v.size()));
      std::istream istream(&mem);
      Reader stream(istream);
      stream >> t;
    }
    template <typename T>
    bool save(const T &t, const std::string &filename)
    {
      try
      {
        std::ofstream dataFile;
        dataFile.open(filename, std::ios_base::binary | std::ios_base::out | std::ios::trunc);
        if (dataFile.fail())
        {

          return false;
        }

        Writer stream(dataFile);

        stream << t;

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

    template <class T>
    bool read(T &t, const std::string &filename)
    {
      try
      {
        std::ifstream dataFile;
        dataFile.open(filename, std::ios_base::binary | std::ios_base::in);
        if (dataFile.fail())
        {
          std::cout << "Fail to open a binary file: " << filename << "!" << std::endl;
          std::cout << "Error Reason: " << strerror(errno) << "!" << std::endl;
          return false;
        }

        Reader stream(dataFile);

        stream >> t;
        return !dataFile.fail();
      }
      catch (std::exception &e)
      {
        std::cout << "Load exception :" << e.what() << std::endl;
        return false;
      }
    }
  }

}
