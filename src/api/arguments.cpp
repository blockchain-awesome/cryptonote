#include "arguments.h"

namespace api
{

Arguments::Arguments(Options *general, Options *parameter)
    : m_general(general), m_parameter(parameter)
{
  m_desc_all.add(*m_general->getDesc()).add(*m_parameter->getDesc());
}

void Arguments::init(int argc, char *argv[])
{
  m_argc = argc;
  m_argv = argv;
}

} // namespace api