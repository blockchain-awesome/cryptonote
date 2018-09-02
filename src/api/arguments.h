

#include "options.h"

namespace po = boost::program_options;

namespace api
{
extern const command_line::arg_descriptor<std::string> arg_address;
extern const command_line::arg_descriptor<std::string> arg_view_key;
extern const command_line::arg_descriptor<std::string> arg_daemon_host;
extern const command_line::arg_descriptor<uint16_t> arg_daemon_port;
extern const command_line::arg_descriptor<uint32_t> arg_log_level;
extern const command_line::arg_descriptor<bool> arg_testnet;

class Arguments
{
public:
  Arguments(Options &general, Options &parameter);
  bool parse();

protected:
  Options &m_general;
  Options &m_parameter;
};

} // namespace api
