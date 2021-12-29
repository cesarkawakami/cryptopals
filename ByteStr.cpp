#include "utils.h"
#include "ByteStr.h"

namespace pals {

int from_hex(char c) {
    if ('0' <= c && c <= '9') {
        return c - '0';
    } else {
        c = std::tolower(c);
        if ('a' <= c && c <= 'f') {
            return c - 'a' + 10;
        }
    }
    fatal("from_hex: invalid char");
}

char to_hex(int c) {
    if (0 <= c && c <= 9) {
        return c + '0';
    } else if (10 <= c && c <= 15) {
        return c - 10 + 'a';
    }
    fatal("to_hex: invalid hex digit");
}

int from_b64(char c) {
    if ('A' <= c && c <= 'Z') {
        return c - 'A';
    } else if ('a' <= c && c <= 'z') {
        return c - 'a' + 26;
    } else if ('0' <= c && c <= '9') {
        return c - '0' + 52;
    } else if (c == '+') {
        return 62;
    } else if (c == '/') {
        return 63;
    }
    fatal("from_b64: invalid char");
}

char to_b64(int c) {
    if (0 <= c && c <= 25) {
        return c + 'A';
    } else if (26 <= c && c <= 51) {
        return c - 26 + 'a';
    } else if (52 <= c && c <= 61) {
        return c - 52 + '0';
    } else if (c == 62) {
        return '+';
    } else if (c == 63) {
        return '/';
    }
    fatal("to_b64: invalid b64 digit");
}

ByteStr ByteStr::from_hex(const std::string &hex) {
    ByteStr rv;
    for (auto it = hex.begin(); it < hex.end(); it += 2) {
        int a = pals::from_hex(*it), b = pals::from_hex(*(it + 1));
        int x = (a << 4) | b;
        rv.data.push_back(static_cast<uint8_t>(x));
    }
    return rv;
}

std::string ByteStr::to_hex() const {
    std::string hex;
    for (uint8_t c : data) {
        int a = c >> 4, b = c & 0xf;
        hex.push_back(pals::to_hex(a));
        hex.push_back(pals::to_hex(b));
    }
    return hex;
}

ByteStr ByteStr::from_b64(const std::string &b64) {
    auto get = [&](auto i) { return (i < std::size(b64)) ? pals::from_b64(b64[i]) : 0; };
    ByteStr rv;
    for (auto i = std::size(b64) * 0; i < std::size(b64); i += 4) {
        int x = (((((get(i) << 6) | get(i + 1)) << 6) | get(i + 2)) << 6) | get(i + 3);
        rv.data.push_back(x >> 16);
        rv.data.push_back((x >> 8) & 0xff);
        rv.data.push_back(x & 0xff);
    }
    return rv;
}

std::string ByteStr::to_b64() const {
    auto get = [&](auto i) { return (i < std::size(data)) ? data[i] : 0; };
    std::string rv;
    for (auto i = std::size(data) * 0; i < std::size(data); i += 3) {
        int x = (((get(i) << 8) | get(i + 1)) << 8) | get(i + 2);
        rv.push_back(pals::to_b64(x >> 18));
        rv.push_back(pals::to_b64((x >> 12) & 0x3f));
        rv.push_back((i + 1 < std::size(data)) ? pals::to_b64((x >> 6) & 0x3f) : '=');
        rv.push_back((i + 2 < std::size(data)) ? pals::to_b64(x & 0x3f) : '=');
    }
    return rv;
}

ByteStr &ByteStr::operator^=(const ByteStr &other) {
    assume(data.size() == other.data.size(), "lengths mismatch");
    for (auto i = data.size() * 0; i < data.size(); ++i) {
        data[i] ^= other.data[i];
    }
    return *this;
}

ByteStr ByteStr::operator^(const ByteStr &other) const {
    ByteStr rv{*this};
    rv ^= other;
    return rv;
}

}; // namespace pals
