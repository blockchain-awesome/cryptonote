

#pragma once

#include "options.h"

namespace po = boost::program_options;

namespace api
{
class Arguments
{
public:
  Arguments(Options *general, Options *parameter);
  void init(int argc, char *argv[]);
  template <typename F, typename G>
  bool parseGeneral(F help, G version)
  {
    return command_line::handle_error_helper(m_desc_all, [&] {
      const auto &desc = *m_general->getDesc();
      const auto &option = command_line::parse_command_line(m_argc, m_argv, desc, true);
      po::store(option, m_vm);
      if (command_line::get_arg(m_vm, command_line::arg_help))
      {
        help();
        std::cout << desc << std::endl;
        return false;
      }
      else if (command_line::get_arg(m_vm, command_line::arg_version))
      {
        version();
        return false;
      }
      return true;
    });
  }
  template <typename F>
  bool parseParameter(F executor)
  {
    return command_line::handle_error_helper(m_desc_all, [&] {
      auto parser = po::command_line_parser(m_argc, m_argv).options(*m_parameter->getDesc()).positional(m_positional_options);
      po::store(parser.run(), m_vm);
      po::notify(m_vm);
      return executor();
    });
  }

protected:
  Options *m_general;
  Options *m_parameter;

  po::positional_options_description m_positional_options;
  po::options_description m_desc_all;
  po::variables_map m_vm;
  int m_argc;
  char **m_argv;
};

} // namespace api
