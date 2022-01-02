#include "ByteStr.h"
#include "attacks_aes_block.h"
#include "prim.h"
#include "range/v3/view/concat.hpp"
#include "range/v3/view/generate_n.hpp"
#include "range/v3/view/iota.hpp"
#include "range/v3/view/join.hpp"
#include "range/v3/view/repeat_n.hpp"
#include "utils.h"
#include <gtest/gtest.h>
#include <random>

using pals::bytestr::ByteStr;
namespace R = ranges::views;

namespace pals::set2_tests {

TEST(ChallengeSet2, Ex9_ImplementPkcs7Padding) {
    const auto input = ByteStr::from_string_raw("YELLOW SUBMARINE");
    const std::string expected = "YELLOW SUBMARINE\x04\x04\x04\x04";
    const auto actual = prim::pkcs7_pad(ByteStr{input}, 20);
    EXPECT_EQ(actual.to_string_raw(), expected);
}

TEST(ChallengeSet2, Ex10_ImplementCBCMode_Encode1) {
    const auto clear_text = ByteStr::from_string_raw("as armas e os baroes assin");
    const auto key = ByteStr::from_string_raw("yellow submarine");
    const auto iv = R::repeat_n(uint8_t{0}, 16) | ranges::to<ByteStr>();
    const std::string expected = "97e452827c24d27ecc77ecd104d9fc9fd08900803589cb2c1b7c0797e89d36b6";
    const auto &actual = prim::aes_cbc_enc(clear_text, key, iv);
    EXPECT_EQ(actual.to_hex(), expected);
}

TEST(ChallengeSet2, Ex10_ImplementCBCMode_Decode1) {
    const auto &cipher_text = ByteStr::from_hex(
        "97e452827c24d27ecc77ecd104d9fc9fd08900803589cb2c1b7c0797e89d36b6");
    const auto &key = ByteStr::from_string_raw("yellow submarine");
    const auto &iv = R::repeat_n(uint8_t{0}, 16) | ranges::to<ByteStr>();
    const std::string &expected = "as armas e os baroes assin";
    const auto &actual = prim::aes_cbc_dec(cipher_text, key, iv);
    EXPECT_EQ(actual.to_string_raw(), expected);
}

TEST(ChallengeSet2, Ex10_ImplementCBCMode_Actual) {
    const auto &cipher_text = ByteStr::from_b64(utils::read_file("challenge-data/10.txt"));
    const auto &key = ByteStr::from_string_raw("YELLOW SUBMARINE");
    const auto &iv = R::repeat_n(uint8_t{0}, 16) | ranges::to<ByteStr>();
    const auto &expected = utils::read_file("challenge-data/6.output.txt");
    const auto &actual = prim::aes_cbc_dec(cipher_text, key, iv);
    EXPECT_EQ(actual.to_string_raw(), expected);
}

TEST(ChallengeSet2, Ex11_AnEcbCbcDetectionOracle) {
    std::mt19937 rnd{std::random_device{}()};
    std::uniform_int_distribution<> bool_dist(0, 1);
    std::uniform_int_distribution<> byte_dist(0, 255);
    std::uniform_int_distribution<> pad_dist(5, 10);

    auto rand_bytes = [&](std::size_t len) {
        return ByteStr{
            R::generate_n([&]() { return static_cast<uint8_t>(byte_dist(rnd)); }, len) |
            ranges::to<std::vector>()};
    };

    auto generate_oracle = [&]() {
        attacks_aes_block::EcbOrCbc which =
            bool_dist(rnd) ? attacks_aes_block::EcbOrCbc::ECB : attacks_aes_block::EcbOrCbc::CBC;
        return std::tuple{
            which,
            [which, &rand_bytes, &pad_dist, &rnd](const ByteStr &input) {
                const auto &key = rand_bytes(16);
                const auto &iv = rand_bytes(16);

                int left_pad_len = pad_dist(rnd);
                int right_pad_len = pad_dist(rnd);
                const auto &left_pad = rand_bytes(left_pad_len);
                const auto &right_pad = rand_bytes(right_pad_len);
                const auto &new_input = R::concat(left_pad, input, right_pad) | ranges::to<ByteStr>();
                if (which == attacks_aes_block::EcbOrCbc::CBC) {
                    return prim::aes_cbc_enc(new_input, key, iv);
                } else if (which == attacks_aes_block::EcbOrCbc::ECB) {
                    return prim::aes_ecb_enc(prim::pkcs7_pad(new_input, 16), key);
                } else {
                    utils::fatal("invalid which?");
                }
            },
        };
    };

    for ([[maybe_unused]] int _ : R::iota(0, 10)) {
        auto [which, oracle] = generate_oracle();
        auto detected = attacks_aes_block::detect_from_encryption_oracle(oracle);
        // std::cerr << "generated=" << static_cast<int>(which) << " detected=" << static_cast<int>(detected) << "\n";
        EXPECT_EQ(which, detected);
    }
}

} // namespace pals::set2_tests
