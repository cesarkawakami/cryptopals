#include "ByteStr.h"
#include "attacks_xor.h"
#include "scoring.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <numeric>
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

} // namespace pals::set1_tests