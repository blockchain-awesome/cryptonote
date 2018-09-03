
#include "options.h"

namespace po = boost::program_options;

namespace api
{

Options::Options(std::string name) : m_name(name)
{
  m_desc_ptr = new po::options_description(name);
}

Options::~Options()
{
  delete m_desc_ptr;
}

void Options::add(const command_line::arg_descriptor<bool, false> &arg)
{
  command_line::add_arg(*m_desc_ptr, arg);
};

void Options::add(const command_line::arg_descriptor<uint16_t, false> &arg)
{
  command_line::add_arg(*m_desc_ptr, arg);
};
void Options::add(const command_line::arg_descriptor<uint32_t, false> &arg)
{
  command_line::add_arg(*m_desc_ptr, arg);
};
void Options::add(const command_line::arg_descriptor<std::string, false> &arg)
{
  command_line::add_arg(*m_desc_ptr, arg);
};

po::options_description* Options::getDesc()
{
  return m_desc_ptr;
}

} // namespace api