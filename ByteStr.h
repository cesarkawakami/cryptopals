#pragma once
#include "utils.h"
#include <vector>

namespace pals {

int from_hex(char c);
char to_hex(int c);

int from_b64(char c);
char to_b64(int c);

struct ByteStr {
    std::vector<uint8_t> data;

    static ByteStr from_hex(const std::string &hex);
    static ByteStr from_b64(const std::string &b64);

    std::string to_hex() const;
    std::string to_b64() const;

    ByteStr &operator^=(const ByteStr &);
    ByteStr operator^(const ByteStr &) const;
};

} // namespace pals
