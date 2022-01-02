#pragma once
#include "utils.h"
#include <vector>

namespace pals::bytestr {

int from_hex(char c);
char to_hex(int c);

int from_b64(char c);
char to_b64(int c);

struct ByteStr {
    std::vector<uint8_t> data;

    static ByteStr from_hex(const std::string &hex);
    static ByteStr from_b64(const std::string &b64);
    static ByteStr from_string_raw(const std::string &s);
    static ByteStr sized(const std::size_t &size);

    std::string to_hex() const;
    std::string to_b64() const;

    ByteStr &operator^=(const ByteStr &);
    ByteStr operator^(const ByteStr &) const;
    ByteStr &operator^=(const uint8_t &);
    ByteStr operator^(const uint8_t &) const;

    std::string to_string_esc() const;
    std::string to_string_raw() const;

    uint8_t &operator[](std::ptrdiff_t i);
    const uint8_t &operator[](std::ptrdiff_t i) const;
    std::size_t size() const;

    std::vector<uint8_t>::iterator begin();
    std::vector<uint8_t>::const_iterator begin() const;
    std::vector<uint8_t>::iterator end();
    std::vector<uint8_t>::const_iterator end() const;
};

} // namespace pals::bytestr
