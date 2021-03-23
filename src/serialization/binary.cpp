

#include <sstream>
#include <iostream>
#include <fstream>
#include "cryptonote.h"
#include "stream/reader.h"
#include "stream/writer.h"

using namespace std;

namespace serializer
{
  class IBinary
  {
    IBinary();

  public:
    virtual void read(std::istream &is) = 0;
    virtual void write(std::ostream &os) const = 0;
    binary_array_t to()
    {
      binary_array_t binaryArray;
      std::ostringstream oss;
      this->write(oss);
      binaryArray.resize(oss.str().length());
      memcpy(&binaryArray[0], oss.str().c_str(), oss.str().length());
      return binaryArray;
    }

    void from(const binary_array_t &blob)
    {
      std::istringstream iss(std::string(blob.begin(), blob.end()));
      this->read(iss);
    }

    bool save(const std::string &filename)
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

    bool load(const std::string &filename)
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

    friend std::istream &operator>>(std::istream &is, IBinary &base)
    {
      base.read(is);
      return is;
    }

    friend std::ostream &operator<<(std::ostream &os, const IBinary &base)
    {
      base.write(os);
      return os;
    }
  };
}