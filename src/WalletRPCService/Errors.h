// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
#include <system_error>

namespace MultiWalletService
{

enum class ErrorCode
{
  OK = 0
};

// custom category:
class ErrorCategory : public std::error_category
{
public:
  static ErrorCategory INSTANCE;

  virtual const char *name() const throw() override
  {
    return "ErrorCategory";
  }

  virtual std::error_condition default_error_condition(int ev) const throw() override
  {
    return std::error_condition(ev, *this);
  }

  virtual std::string message(int ev) const override
  {
    ErrorCode code = static_cast<ErrorCode>(ev);

    switch (code)
    {
    case ErrorCode::OK:
      return "OK";
    default:
      return "Unknown error";
    }
  }

private:
  ErrorCategory()
  {
  }
};

} // namespace MultiWalletService

inline std::error_code make_error_code(MultiWalletService::ErrorCode e)
{
  return std::error_code(static_cast<int>(e), MultiWalletService::ErrorCategory::INSTANCE);
}

namespace std
{

template <>
struct is_error_code_enum<MultiWalletService::ErrorCode> : public true_type
{
};

} // namespace std
