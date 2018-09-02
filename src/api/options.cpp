
#include "options.h"

namespace api
{

Options::Options(std::string name) : m_name(name)
{
  m_desc_ptr = std::unique_ptr<po::options_description>(new po::options_description(name));
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
} // namespace api