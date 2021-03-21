
#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring> // memcpy
#include <vector>

#include "reader.h"
#include "writer.h"

namespace Common
{


  // struct membuf : std::streambuf
  // {
  //   membuf(char *begin, char *end)
  //   {
  //     this->setg(begin, begin, end);
  //   }
  // };

  // class Reader : public Reader
  // {
  // public:
  //   Reader(const void *buffer, size_t bufferSize);
  //   size_t getPosition() const;
  //   bool endOfStream() const;

  //   // IInputStream
  //   // virtual size_t readSome(void *data, size_t size) override;

  // private:
  //   const char *buffer;
  //   size_t bufferSize;
  //   size_t position;
  //   std::istream temp;
  //   membuf mem;
  // };

  class MemoryOutputStream : public Writer
  {
  public:
    MemoryOutputStream() : Writer(), m_writePos(0)
    {
    }

    virtual size_t writeSome(const void *data, size_t size) override
    {
      if (size == 0)
      {
        return 0;
      }

      if (m_writePos + size > m_buffer.size())
      {
        m_buffer.resize(m_writePos + size);
      }

      memcpy(&m_buffer[m_writePos], data, size);
      m_writePos += size;
      return size;
    }

    size_t size()
    {
      return m_buffer.size();
    }

    const uint8_t *data()
    {
      return m_buffer.data();
    }

    void clear()
    {
      m_writePos = 0;
      m_buffer.resize(0);
    }

  private:
    size_t m_writePos;
    std::vector<uint8_t> m_buffer;
  };

}