
#include "ByteStr.h"
#include "prim.h"
#include <gtest/gtest.h>

namespace pals::set2_tests {

TEST(ChallengeSet2, Ex1_ImplementPkcs7Padding) {
    const auto input = bytestr::ByteStr::from_string_raw("YELLOW SUBMARINE");
    const std::string expected = "YELLOW SUBMARINE\x04\x04\x04\x04";
    const auto actual = prim::pkcs7(bytestr::ByteStr{input}, 20);
    EXPECT_EQ(actual.to_string_raw(), expected);
}

} // namespace pals::set2_tests
