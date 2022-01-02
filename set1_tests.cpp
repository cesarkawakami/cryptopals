#include "ByteStr.h"
#include "attacks_xor.h"
#include "prim.h"
#include "range/v3/algorithm/for_each.hpp"
#include "range/v3/range/conversion.hpp"
#include "range/v3/view/chunk.hpp"
#include "range/v3/view/istream.hpp"
#include "range/v3/view/transform.hpp"
#include "scoring.h"
#include "utils.h"
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <numeric>
#include <rijndael.h>
#include <sstream>

using pals::bytestr::ByteStr;
using pals::scoring::AsciiUnigramAnalyzer;
using pals::scoring::LetterUnigramAnalyzer;
namespace R = ranges::views;

namespace pals::set1_tests {

TEST(ChallengeSet1, Ex1_HexToBase64) {
    const std::string input = "49276d206b696c6c696e6720796f757220627261696e206c696b65206120706f69736f6e6f7573206d757368726f6f6d";
    const std::string expected = "SSdtIGtpbGxpbmcgeW91ciBicmFpbiBsaWtlIGEgcG9pc29ub3VzIG11c2hyb29t";
    const std::string actual = ByteStr::from_hex(input).to_b64();
    EXPECT_EQ(actual, expected);
}

TEST(ChallengeSet1, Ex2_FixedXOR) {
    const std::string input1 = "1c0111001f010100061a024b53535009181c";
    const std::string input2 = "686974207468652062756c6c277320657965";
    const std::string expected = "746865206b696420646f6e277420706c6179";
    const std::string actual = (ByteStr::from_hex(input1) ^ ByteStr::from_hex(input2)).to_hex();
    EXPECT_EQ(actual, expected);
}

TEST(ChallengeSet1, Ex3_SingleByteXORCypher) {
    const std::string input = "1b37373331363f78151b7f2b783431333d78397828372d363c78373e783a393b3736";

    auto cypher_text = ByteStr::from_hex(input);
    const auto &[key, score] = attacks_xor::solve_xor1(cypher_text);
    auto decoded = cypher_text ^ key;
    auto output = decoded.to_string_esc();
    EXPECT_EQ(output, "Cooking MC's like a pound of bacon");
}

TEST(ChallengeSet1, Ex4_DetectSingleCharacterXOR) {
    std::ifstream fin{"challenge-data/4.txt"};
    const auto &[tpl, score] = utils::best_element(
        ranges::istream<std::string>(fin) |
            R::transform([](const auto &line) {
                const auto &cypher_text = ByteStr::from_hex(line);
                const auto &[key, score] = attacks_xor::solve_xor1(cypher_text);
                return std::tuple{cypher_text, key, score};
            }),
        [](const auto &t) { const auto &[ct, k, s] = t; return s; },
        std::greater{});

    const auto &[cypher_text, key, score2] = tpl;
    const auto clear_text = cypher_text ^ key;
    EXPECT_EQ(clear_text.to_string_raw(), "Now that the party is jumping\n");
}

TEST(ChallengeSet1, Ex5_ImplementRepeatingKeyXOR) {
    const std::string input{"Burning 'em, if you ain't quick and nimble\n"
                            "I go crazy when I hear a cymbal"};
    const std::string key{"ICE"};
    const std::string expected{"0b3637272a2b2e63622c2e69692a23693a2a3c6324202d623d63343c2a26226324272765272"
                               "a282b2f20430a652e2c652a3124333a653e2b2027630c692b20283165286326302e27282f"};

    ByteStr byte_str{ByteStr::from_string_raw(input)};
    for (auto i = byte_str.data.size() * 0; i < byte_str.data.size(); ++i) {
        byte_str.data[i] ^= static_cast<uint8_t>(key[i % key.size()]);
    }
    ASSERT_EQ(byte_str.to_hex(), expected);
}

TEST(ChallengeSet1, Ex6_BreakRepeatingKeyXORKeySize) {
    const auto cypher_text = ByteStr::from_b64(utils::read_file("challenge-data/6.txt"));

    double best_dist = std::numeric_limits<double>::infinity();
    int best_key_size = -1;
    for (int key_size = 1; key_size < 40; ++key_size) {
        double total_pairs = 0;
        double total_distance = 0;
        for (int i = 0; i < key_size; ++i) {
            for (const auto &p : cypher_text | R::drop(i) | R::stride(key_size) | R::chunk(2)) {
                if (p.size() != 2) {
                    continue;
                }
                const auto &a = *p.begin(), &b = *(p.begin() + 1);
                total_distance += __builtin_popcount(a ^ b);
                ++total_pairs;
            }
        }
        double norm_distance = total_distance / total_pairs;
        // std::cerr << "ks=" << key_size << " pairs=" << total_pairs << " dist=" << norm_distance << "\n";
        if (norm_distance < best_dist) {
            best_dist = norm_distance;
            best_key_size = key_size;
        }
    }

    EXPECT_EQ(best_key_size, 29);
}

TEST(ChallengeSet1, Ex6_BreakRepeatingKeyXOR) {
    const auto cypher_text = ByteStr::from_b64(utils::read_file("challenge-data/6.txt"));
    const auto expected = utils::read_file("challenge-data/6.output.txt");

    double best_score = -std::numeric_limits<double>::infinity();
    ByteStr best_key;

    for (int key_size = 1; key_size <= 40; ++key_size) {
        auto key = ByteStr::sized(key_size);
        for (int i = 0; i < key_size; ++i) {
            ByteStr strided{cypher_text | R::drop(i) | R::stride(key_size) | ranges::to<std::vector>()};
            key[i] = std::get<0>(attacks_xor::solve_xor1(strided));
        }

        auto clear_text{cypher_text};
        for (auto i = clear_text.data.size() * 0; i < clear_text.data.size(); ++i) {
            clear_text[i] ^= key[i % key.size()];
        }
        double score = scoring::score1(clear_text);
        if (score > best_score) {
            best_score = score;
            best_key = std::move(key);
        }
    }

    auto clear_text{cypher_text};
    for (auto i = clear_text.data.size() * 0; i < clear_text.data.size(); ++i) {
        clear_text.data[i] ^= best_key[i % best_key.size()];
    }

    const std::string actual{clear_text.to_string_raw()};
    EXPECT_EQ(actual, expected);
}

TEST(ChallengeSet1, Ex7_AesInEcbMode) {
    const auto cipher_text = ByteStr::from_b64(utils::read_file("challenge-data/7.txt"));
    const auto key = ByteStr::from_string_raw("YELLOW SUBMARINE");
    const auto actual = prim::pkcs7_unpad(prim::aes_ecb_dec(cipher_text, key));
    const auto expected = utils::read_file("challenge-data/6.output.txt");
    EXPECT_EQ(actual.to_string_raw(), expected);
}

TEST(ChallengeSet1, Ex8_DetectAesInEcbMode) {
    std::ifstream fin{"challenge-data/8.txt"};
    const std::vector<ByteStr> cipher_texts =
        ranges::istream<std::string>(fin) |
        R::transform([](const std::string &s) { return ByteStr::from_string_raw(s); }) |
        ranges::to<std::vector>();

    auto has_repetition = [](const ByteStr &bs) {
        std::set<std::vector<uint8_t>> seen;
        for (const auto &r : bs | R::chunk(16)) {
            const auto v = r | ranges::to<std::vector>();
            if (seen.contains(v)) {
                return true;
            }
            seen.insert(v);
        }
        return false;
    };

    auto candidates = cipher_texts | R::filter(has_repetition) | ranges::to<std::vector>();
    utils::expect(candidates.size() == 1, "oh no");
    const std::string expected =
        "d880619740a8a19b7840a8a31c810a3d08649af70dc06f4fd5d2d69c744cd283e2dd052f6b641dbf9d11b0348"
        "542bb5708649af70dc06f4fd5d2d69c744cd2839475c9dfdbc1d46597949d9c7e82bf5a08649af70dc06f4fd5"
        "d2d69c744cd28397a93eab8d6aecd566489154789a6b0308649af70dc06f4fd5d2d69c744cd283d403180c98c"
        "8f6db1f2a3f9c4040deb0ab51b29933f2c123c58386b06fba186a";
    EXPECT_EQ(candidates[0].to_string_esc(), expected);
}

} // namespace pals::set1_tests
