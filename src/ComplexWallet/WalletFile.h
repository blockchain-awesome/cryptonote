#pragma once

#include <iostream>
#include <string>

#include <Logging/LoggerRef.h>


extern bool create_wallet_by_keys(std::string &wallet_file, std::string &password,
                                  std::string &address, std::string &spendKey,
                                  std::string &viewKey,
                                  Logging::LoggerRef &logger);
