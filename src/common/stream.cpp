#include "stream.h"
#include <string> 
#include <fstream>

namespace stream {
bool load(const std::string& filepath, std::string& buf) {
  try {
    std::ifstream fstream;
    fstream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fstream.open(filepath, std::ios_base::binary | std::ios_base::in | std::ios::ate);

    size_t fileSize = static_cast<size_t>(fstream.tellg());
    buf.resize(fileSize);

    if (fileSize > 0)  {
      fstream.seekg(0, std::ios::beg);
      fstream.read(&buf[0], buf.size());
    }
  } catch (const std::exception&) {
    return false;
  }

  return true;
}

bool save(const std::string& filepath, const std::string& buf) {
  try {
    std::ofstream fstream;
    fstream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fstream.open(filepath, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
    fstream << buf;
  } catch (const std::exception&)  {
    return false;
  }

  return true;
}
}