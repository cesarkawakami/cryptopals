#include "ByteStr.h"
#include "utils.h"
#include <cctype>
#include <gtest/gtest.h>
#include <iomanip>
#include <sstream>

namespace pals::bytestr {

int from_hex(char c) {
    if ('0' <= c && c <= '9') {
        return c - '0';
    } else {
        c = std::tolower(c);
        if ('a' <= c && c <= 'f') {
            return c - 'a' + 10;
        }
    }
    utils::fatal("from_hex: invalid char");
}

char to_hex(int c) {
    if (0 <= c && c <= 9) {
        return c + '0';
    } else if (10 <= c && c <= 15) {
        return c - 10 + 'a';
    }
    utils::fatal("to_hex: invalid hex digit");
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
    } else if (c == '=') {
        return 0;
    }
    utils::fatal("from_b64: invalid char");
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
    utils::fatal("to_b64: invalid b64 digit");
}

ByteStr ByteStr::from_hex(const std::string &input_hex) {
    using pals::bytestr::from_hex;
    std::string hex;
    std::copy_if(input_hex.begin(), input_hex.end(), std::back_inserter(hex), [](char c) {
        return ('0' <= c && c <= '9') || ('a' <= std::tolower(c) && std::tolower(c) <= 'f');
    });
    utils::expect(hex.size() % 2 == 0, "hex string must have even length");
    ByteStr rv;
    for (auto it = hex.begin(); it < hex.end(); it += 2) {
        int a = from_hex(*it), b = from_hex(*(it + 1));
        int x = (a << 4) | b;
        rv.data.push_back(static_cast<uint8_t>(x));
    }
    return rv;
}

std::string ByteStr::to_hex() const {
    using pals::bytestr::to_hex;
    std::string hex;
    for (uint8_t c : data) {
        int a = c >> 4, b = c & 0xf;
        hex.push_back(to_hex(a));
        hex.push_back(to_hex(b));
    }
    return hex;
}

ByteStr ByteStr::from_b64(const std::string &input_b64) {
    using pals::bytestr::from_b64;
    std::string b64;
    std::copy_if(input_b64.begin(), input_b64.end(), std::back_inserter(b64), [](char c) {
        return ('a' <= std::tolower(c) && std::tolower(c) <= 'z') ||
               ('0' <= c && c <= '9') ||
               c == '+' || c == '/' || c == '=';
    });
    utils::expect(b64.size() % 4 == 0, "base64 string must have length multiple of 4");
    ByteStr rv;
    for (auto i = std::size(b64) * 0; i < std::size(b64); i += 4) {
        int x = 0;
        x = (x << 6) | from_b64(b64[i]);
        x = (x << 6) | from_b64(b64[i + 1]);
        x = (x << 6) | from_b64(b64[i + 2]);
        x = (x << 6) | from_b64(b64[i + 3]);
        rv.data.push_back(x >> 16);
        if (b64[i + 2] != '=') {
            rv.data.push_back((x >> 8) & 0xff);
        }
        if (b64[i + 3] != '=') {
            rv.data.push_back(x & 0xff);
        }
    }
    return rv;
}

std::string ByteStr::to_b64() const {
    using pals::bytestr::to_b64;
    auto get = [&](auto i) { return (i < std::size(data)) ? data[i] : 0; };
    std::string rv;
    for (auto i = std::size(data) * 0; i < std::size(data); i += 3) {
        int x = 0;
        x = (x << 8) | get(i);
        x = (x << 8) | get(i + 1);
        x = (x << 8) | get(i + 2);
        rv.push_back(to_b64(x >> 18));
        rv.push_back(to_b64((x >> 12) & 0x3f));
        rv.push_back((i + 1 < std::size(data)) ? to_b64((x >> 6) & 0x3f) : '=');
        rv.push_back((i + 2 < std::size(data)) ? to_b64(x & 0x3f) : '=');
    }
    return rv;
}

ByteStr &ByteStr::operator^=(const ByteStr &other) {
    utils::expect(data.size() == other.data.size(), "lengths mismatch");
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

ByteStr &ByteStr::operator^=(const uint8_t &other) {
    for (auto &c : data) {
        c ^= other;
    }
    return *this;
}

ByteStr ByteStr::operator^(const uint8_t &other) const {
    ByteStr rv{*this};
    rv ^= other;
    return rv;
}

ByteStr ByteStr::from_string_raw(const std::string &s) {
    ByteStr rv;
    for (char c : s) {
        rv.data.push_back(static_cast<uint8_t>(c));
    }
    return rv;
}

ByteStr ByteStr::sized(const std::size_t &size) {
    return ByteStr{std::vector<uint8_t>(size)};
}

std::string ByteStr::to_string_esc() const {
    std::ostringstream ss;
    for (auto ch_ : data) {
        char ch = ch_;
        if (' ' <= ch && ch <= '~') {
            ss << ch;
        } else if (ch == '\n') {
            ss << "\\n";
        } else {
            ss << "\\x" << std::setw(2) << std::setfill('0') << std::hex << int{ch_};
        }
    }
    return ss.str();
}

std::string ByteStr::to_string_raw() const {
    std::string rv;
    for (auto ch : data) {
        rv.push_back(static_cast<char>(ch));
    }
    return rv;
}

uint8_t &ByteStr::operator[](std::ptrdiff_t i) {
    return data[i];
}
const uint8_t &ByteStr::operator[](std::ptrdiff_t i) const {
    return data[i];
}
std::size_t ByteStr::size() const { return data.size(); }
std::vector<uint8_t>::iterator ByteStr::begin() {
    return data.begin();
}
std::vector<uint8_t>::const_iterator ByteStr::begin() const {
    return data.begin();
}
std::vector<uint8_t>::iterator ByteStr::end() {
    return data.end();
}
std::vector<uint8_t>::const_iterator ByteStr::end() const {
    return data.end();
}

TEST(ByteStr, Base64Output1) {
    const std::string input = "Many hands make light work.";
    const std::string expected = "TWFueSBoYW5kcyBtYWtlIGxpZ2h0IHdvcmsu";
    const std::string actual = ByteStr::from_string_raw(input).to_b64();
    EXPECT_EQ(actual, expected);
}

TEST(ByteStr, Base64Output2) {
    const std::string input = "Many hands make light work.T";
    const std::string expected = "TWFueSBoYW5kcyBtYWtlIGxpZ2h0IHdvcmsuVA==";
    const std::string actual = ByteStr::from_string_raw(input).to_b64();
    EXPECT_EQ(actual, expected);
}

TEST(ByteStr, Base64Output3) {
    const std::string input = "Many hands make light work.TY";
    const std::string expected = "TWFueSBoYW5kcyBtYWtlIGxpZ2h0IHdvcmsuVFk=";
    const std::string actual = ByteStr::from_string_raw(input).to_b64();
    EXPECT_EQ(actual, expected);
}

TEST(ByteStr, Base64Input1) {
    const std::string input = "TWFueSBoYW5kcyBtYWtlIGxpZ2h0IHdvcmsu";
    const std::string expected = "Many hands make light work.";
    const std::string actual = ByteStr::from_b64(input).to_string_raw();
    EXPECT_EQ(actual, expected);
}

TEST(ByteStr, Base64Input2) {
    const std::string input = "TWFueSBoYW5kcyBtYWtlIGxpZ2h0IHdvcmsuVA==";
    const std::string expected = "Many hands make light work.T";
    const std::string actual = ByteStr::from_b64(input).to_string_raw();
    EXPECT_EQ(actual, expected);
}

TEST(ByteStr, Base64Input3) {
    const std::string input = "TWFueSBoYW5kcyBtYWtlIGxpZ2h0IHdvcmsuVFk=";
    const std::string expected = "Many hands make light work.TY";
    const std::string actual = ByteStr::from_b64(input).to_string_raw();
    EXPECT_EQ(actual, expected);
}

TEST(ByteStr, Base64Input4) {
    const std::string input = "TWFueSBoYW5kcy\nBtYWtlIGxp\nZ2h  0IHdvcm\n\nsuVFk=";
    const std::string expected = "Many hands make light work.TY";
    const std::string actual = ByteStr::from_b64(input).to_string_raw();
    EXPECT_EQ(actual, expected);
}

}; // namespace pals::bytestr
