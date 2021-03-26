// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <limits>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

namespace varint
{

    // template <typename T>
    // std::string from(const T &t)
    // {
    //     std::ostringstream ss;
    //     uint64_t v = t;
    //     while (v >= 0x80)
    //     {
    //         uint8_t tag = (static_cast<char>(v) & 0x7f) | 0x80;
    //         ss << tag;
    //         v >>= 7;
    //     }
    //     uint8_t tag = static_cast<char>(v);
    //     ss << tag;

    //     return ss.str();
    // }

    template <int bits, typename InputIt, typename T>
    typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value && 0 <= bits && bits <= std::numeric_limits<T>::digits, int>::type
    read(InputIt &&first, InputIt &&last, T &i)
    {
        int read = 0;
        i = 0;
        for (int shift = 0;; shift += 7)
        {
            if (first == last)
            {
                return read; // End of input.
            }
            unsigned char byte = *first++;
            ++read;
            if (shift + 7 >= bits && byte >= 1 << (bits - shift))
            {
                return -1; // Overflow.
            }
            if (byte == 0 && shift != 0)
            {
                return -2; // Non-canonical representation.
            }
            i |= static_cast<T>(byte & 0x7f) << shift;
            if ((byte & 0x80) == 0)
            {
                break;
            }
        }
        return read;
    }

    template <typename InputIt, typename T>
    int read(InputIt &&first, InputIt &&last, T &i)
    {
        return read<std::numeric_limits<T>::digits, InputIt, T>(std::move(first), std::move(last), i);
    }
}
