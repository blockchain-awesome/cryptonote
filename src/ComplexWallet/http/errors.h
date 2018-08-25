// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
#include <system_error>

using namespace std;

namespace Errors
{

enum MultiWalletErrorCode
{
  INVALID_ADDRESS = 1,
  INVALID_SEND_SECRET_KEY,
  INVALID_VIEW_SECRET_KEY,
  INVALID_TOKEN,

  MISSING_ADDRESS = 1 << 4,
  MISSING_SEND_SECRET_KEY,
  MISSING_VIEW_SECRET_KEY,
  MISSING_TOKEN,
};

// custom category:
class MultiWalletErrorCategory : public std::error_category
{
public:
  virtual const char *name() const throw() override
  {
    return "MultiWalletErrorCategory";
  }

  static MultiWalletErrorCategory INSTANCE;

  virtual std::error_condition default_error_condition(int ev) const throw() override
  {
    return std::error_condition(ev, *this);
  }

  virtual std::string message(int ev) const override
  {
    MultiWalletErrorCode code = static_cast<MultiWalletErrorCode>(ev);

    switch (code)
    {

    case MultiWalletErrorCode::INVALID_ADDRESS:
      return "Invalid address";
    case MultiWalletErrorCode::INVALID_SEND_SECRET_KEY:
      return "Invalid send secret key";
    case MultiWalletErrorCode::INVALID_VIEW_SECRET_KEY:
      return "Invalid view secret key";
    case MultiWalletErrorCode::INVALID_TOKEN:
      return "Invalid token";
    case MultiWalletErrorCode::MISSING_ADDRESS:
      return "Missing address";
    case MultiWalletErrorCode::MISSING_SEND_SECRET_KEY:
      return "Missing send secret key";
    case MultiWalletErrorCode::MISSING_VIEW_SECRET_KEY:
      return "Missing view secret key";
    case MultiWalletErrorCode::MISSING_TOKEN:
      return "Missing token";
    default:
      return "Unknown error";
    }
  }

private:
  MultiWalletErrorCategory()
  {
  }
};

} // namespace Errors

inline std::error_code make_error_code(Errors::MultiWalletErrorCode e)
{
  return std::error_code(static_cast<int>(e), Errors::MultiWalletErrorCategory::INSTANCE);
}
namespace std
{

template <>
struct is_error_code_enum<Errors::MultiWalletErrorCode> : public true_type
{
};

} // namespace std
