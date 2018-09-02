#pragma once

#include <boost/variant.hpp>
#include "Common/CommandLine.h"

namespace po = boost::program_options;

namespace api
{

class Options
{
public:
  Options(std::string name);
  void add(const command_line::arg_descriptor<bool, false> &arg);
  void add(const command_line::arg_descriptor<uint16_t, false> &arg);
  void add(const command_line::arg_descriptor<uint32_t, false> &arg);
  void add(const command_line::arg_descriptor<std::string, false> &arg);

protected:
  std::string m_name;
  std::unique_ptr<po::options_description> m_desc_ptr;
};

} // namespace api