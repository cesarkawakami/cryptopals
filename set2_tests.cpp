
#include "ByteStr.h"
#include "prim.h"
#include "range/v3/view/repeat_n.hpp"
#include "utils.h"
#include <gtest/gtest.h>

namespace R = ranges::views;

namespace pals::set2_tests {

TEST(ChallengeSet2, Ex1_ImplementPkcs7Padding) {
    const auto input = bytestr::ByteStr::from_string_raw("YELLOW SUBMARINE");
    const std::string expected = "YELLOW SUBMARINE\x04\x04\x04\x04";
    const auto actual = prim::pkcs7_pad(bytestr::ByteStr{input}, 20);
    EXPECT_EQ(actual.to_string_raw(), expected);
}

TEST(ChallengeSet2, Ex2_ImplementCBCMode_Encode1) {
    const auto clear_text = bytestr::ByteStr::from_string_raw("as armas e os baroes assin");
    const auto key = bytestr::ByteStr::from_string_raw("yellow submarine");
    const auto iv = bytestr::ByteStr{R::repeat_n(uint8_t{0}, 16) | ranges::to<std::vector>()};
    const std::string expected = "97e452827c24d27ecc77ecd104d9fc9fd08900803589cb2c1b7c0797e89d36b6";
    const auto &actual = prim::aes_cbc_enc(clear_text, key, iv);
    EXPECT_EQ(actual.to_hex(), expected);
}

TEST(ChallengeSet2, Ex2_ImplementCBCMode_Decode1) {
    const auto &cipher_text = bytestr::ByteStr::from_hex(
        "97e452827c24d27ecc77ecd104d9fc9fd08900803589cb2c1b7c0797e89d36b6");
    const auto &key = bytestr::ByteStr::from_string_raw("yellow submarine");
    const auto &iv = bytestr::ByteStr{R::repeat_n(uint8_t{0}, 16) | ranges::to<std::vector>()};
    const std::string &expected = "as armas e os baroes assin";
    const auto &actual = prim::aes_cbc_dec(cipher_text, key, iv);
    EXPECT_EQ(actual.to_string_raw(), expected);
}

TEST(ChallengeSet2, Ex2_ImplementCBCMode_Actual) {
    const auto &cipher_text = bytestr::ByteStr::from_b64(utils::read_file("challenge-data/10.txt"));
    const auto &key = bytestr::ByteStr::from_string_raw("YELLOW SUBMARINE");
    const auto &iv = bytestr::ByteStr{R::repeat_n(uint8_t{0}, 16) | ranges::to<std::vector>()};
    const auto &expected = utils::read_file("challenge-data/6.output.txt");
    const auto &actual = prim::aes_cbc_dec(cipher_text, key, iv);
    EXPECT_EQ(actual.to_string_raw(), expected);
}

} // namespace pals::set2_tests
