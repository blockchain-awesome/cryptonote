// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "common.h"

namespace command_line
{

namespace po = boost::program_options;
extern const arg_descriptor<bool> arg_print_genesis_tx;
extern const arg_descriptor<std::string> arg_data_dir;
extern const arg_descriptor<std::string> arg_config_file;

class Daemon
{
public:
  Daemon();
  po::options_description desc_cmd_sett;
  po::options_description desc_cmd_only;
  po::options_description desc_options;
  po::variables_map vm;

  void help();
  void prinGenesisTxHex();
  std::string getGenesisName();
  bool checkVersion();

  // Getting parameters
  bool getTestMode();
  std::string getLogFile();
  int getLogLevel();
  bool getConsole();

  template <typename T>
  bool inernalParse(T f)
  {
    std::string data_dir = get_arg(vm, arg_data_dir);
    std::string config = get_arg(vm, arg_config_file);

    boost::filesystem::path data_dir_path(data_dir);
    boost::filesystem::path config_path(config);
    if (!config_path.has_parent_path())
    {
      config_path = data_dir_path / config_path;
    }

    boost::system::error_code ec;
    if (boost::filesystem::exists(config_path, ec))
    {
      po::store(po::parse_config_file<char>(config_path.string<std::string>().c_str(), desc_cmd_sett), vm);
    }
    po::notify(vm);
    return true;
  };

  template <typename T>
  bool parse(int argc, char *argv[], T f)
  {
    try
    {
      po::store(po::parse_command_line(argc, argv, desc_options), vm);
      if (get_arg(vm, arg_help))
      {
        help();
        return false;
      }
      if (get_arg(vm, arg_print_genesis_tx))
      {
        prinGenesisTxHex();
        return false;
      }
      return inernalParse(f);
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

  void assign();

  std::string configFolder;
  bool configFolderDefaulted = true;
};

} // namespace command_line
