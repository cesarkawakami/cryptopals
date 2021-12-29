#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include "ByteStr.h"

TEST(HexAndBase64Test, HexToBase64) {
    const std::string input = "49276d206b696c6c696e6720796f757220627261696e206c696b65206120706f69736f6e6f7573206d757368726f6f6d";
    const std::string expected = "SSdtIGtpbGxpbmcgeW91ciBicmFpbiBsaWtlIGEgcG9pc29ub3VzIG11c2hyb29t";
    const std::string actual = pals::ByteStr::from_hex(input).to_b64();
    EXPECT_EQ(actual, expected);
}

TEST(HexAndBase64Test, FixedXOR) {
    const std::string input1 = "1c0111001f010100061a024b53535009181c";
    const std::string input2 = "686974207468652062756c6c277320657965";
    const std::string expected = "746865206b696420646f6e277420706c6179";
    const std::string actual = (pals::ByteStr::from_hex(input1) ^ pals::ByteStr::from_hex(input2)).to_hex();
    EXPECT_EQ(actual, expected);
}
