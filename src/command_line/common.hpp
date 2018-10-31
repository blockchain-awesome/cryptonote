// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <iostream>
#include <type_traits>

#include <boost/program_options/parsers.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

namespace command_line
{
namespace po = boost::program_options;

template <typename T, bool required = false>
struct arg_descriptor;

template <typename T>
struct arg_descriptor<T, false>
{
  typedef T value_type;

  const char *name;
  const char *description;
  T default_value;
  bool not_use_default;
};

template <typename T>
struct arg_descriptor<std::vector<T>, false>
{
  typedef std::vector<T> value_type;

  const char *name;
  const char *description;
};

template <typename T>
struct arg_descriptor<T, true>
{
  static_assert(!std::is_same<T, bool>::value, "Boolean switch can't be required");

  typedef T value_type;

  const char *name;
  const char *description;
};

template <typename T>
po::typed_value<T, char> *make_semantic(const arg_descriptor<T, true> & /*arg*/)
{
  return po::value<T>()->required();
}

template <typename T>
po::typed_value<T, char> *make_semantic(const arg_descriptor<T, false> &arg)
{
  auto semantic = po::value<T>();
  if (!arg.not_use_default)
    semantic->default_value(arg.default_value);
  return semantic;
}

template <typename T>
po::typed_value<T, char> *make_semantic(const arg_descriptor<T, false> &arg, const T &def)
{
  auto semantic = po::value<T>();
  if (!arg.not_use_default)
    semantic->default_value(def);
  return semantic;
}

template <typename T>
po::typed_value<std::vector<T>, char> *make_semantic(const arg_descriptor<std::vector<T>, false> & /*arg*/)
{
  auto semantic = po::value<std::vector<T>>();
  semantic->default_value(std::vector<T>(), "");
  return semantic;
}

template <typename T, bool required>
void add_arg(po::options_description &description, const arg_descriptor<T, required> &arg, bool unique = true)
{
  if (unique && 0 != description.find_nothrow(arg.name, false))
  {
    std::cerr << "Argument already exists: " << arg.name << std::endl;
    return;
  }

  description.add_options()(arg.name, make_semantic(arg), arg.description);
}

template <typename T>
void add_arg(po::options_description &description, const arg_descriptor<T, false> &arg, const T &def, bool unique = true)
{
  if (unique && 0 != description.find_nothrow(arg.name, false))
  {
    std::cerr << "Argument already exists: " << arg.name << std::endl;
    return;
  }

  description.add_options()(arg.name, make_semantic(arg, def), arg.description);
}

template <>
inline void add_arg(po::options_description &description, const arg_descriptor<bool, false> &arg, bool unique)
{
  if (unique && 0 != description.find_nothrow(arg.name, false))
  {
    std::cerr << "Argument already exists: " << arg.name << std::endl;
    return;
  }

  description.add_options()(arg.name, po::bool_switch(), arg.description);
}

template <typename charT>
po::basic_parsed_options<charT> parse_command_line(int argc, const charT *const argv[],
                                                   const po::options_description &desc, bool allow_unregistered = false)
{
  auto parser = po::command_line_parser(argc, argv);
  parser.options(desc);
  if (allow_unregistered)
  {
    parser.allow_unregistered();
  }
  return parser.run();
}

template <typename F>
bool handle_error_helper(const po::options_description &desc, F parser)
{
  try
  {
    return parser();
  }
  catch (std::exception &e)
  {
    std::cerr << "Failed to parse arguments: " << e.what() << std::endl;
    std::cerr << desc << std::endl;
    return false;
  }
  catch (...)
  {
    std::cerr << "Failed to parse arguments: unknown exception" << std::endl;
    std::cerr << desc << std::endl;
    return false;
  }
}

template <typename T, bool required>
bool has_arg(const po::variables_map &vm, const arg_descriptor<T, required> &arg)
{
  auto value = vm[arg.name];
  return !value.empty();
}

template <typename T, bool required>
T get_arg(const po::variables_map &vm, const arg_descriptor<T, required> &arg)
{
  return vm[arg.name].template as<T>();
}

template <>
inline bool has_arg<bool, false>(const po::variables_map &vm, const arg_descriptor<bool, false> &arg)
{
  return get_arg<bool, false>(vm, arg);
}

typedef struct
{
  std::string full;
  std::string command;
  std::string setting;
} OptionsNames;

class CommandParser
{
public:
  CommandParser(OptionsNames &names) : desc_options(names.full),
                                       desc_cmd_only(names.command), desc_cmd_sett(names.setting){};
  void setup()
  {
    desc_options.add(desc_cmd_only).add(desc_cmd_sett);
  }

  template <typename T>
  bool isDefaulted(T &t)
  {
    return vm[t.name].defaulted();
  }

  template <typename T>
  bool count(T &t)
  {
    return vm.count(t.name);
  }

  template <typename T, bool required>
  T get(const arg_descriptor<T, required> &arg)
  {
    return vm[arg.name].template as<T>();
  }

  virtual bool innerParse() = 0;
  template <typename T>
  void addCommand(const T &arg)
  {
    add_arg(desc_cmd_only, arg);
  };

  template <typename T>
  void addCommand(const T &arg, std::string desc)
  {
    add_arg(desc_cmd_only, arg, desc);
  };

  template <typename T>
  void addSetting(const T &arg)
  {
    add_arg(desc_cmd_sett, arg);
  };

  void notify()
  {
    po::notify(vm);
  }

  template <typename T>
  bool parse(int argc, char *argv[], T f)
  {
    try
    {
      po::store(po::parse_command_line(argc, argv, desc_options), vm);
      if (!innerParse())
      {
        return false;
      }
      notify();
      return f();
    }
    catch (std::exception &e)
    {
      std::cerr << "Failed to parse arguments: " << e.what() << std::endl;
      std::cerr << desc_options << std::endl;
      return false;
    }
    catch (...)
    {
      std::cerr << "Failed to parse arguments: unknown exception" << std::endl;
      std::cerr << desc_options << std::endl;
      return false;
    }
  };

  po::options_description desc_cmd_sett;
  po::options_description desc_cmd_only;
  po::options_description desc_options;
  po::variables_map vm;
};

} // namespace command_line
