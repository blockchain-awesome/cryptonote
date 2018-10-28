#pragma once

#include <boost/variant.hpp>
#include "command_line/common.h"

namespace po = boost::program_options;

namespace api
{

class Options
{
public:
  Options(std::string name);
  ~Options();
  void add(const command_line::arg_descriptor<bool, false> &arg);
  void add(const command_line::arg_descriptor<uint16_t, false> &arg);
  void add(const command_line::arg_descriptor<uint32_t, false> &arg);
  void add(const command_line::arg_descriptor<std::string, false> &arg);
  po::options_description* getDesc();

protected:
  std::string m_name;
  po::options_description* m_desc_ptr;
};

} // namespace api