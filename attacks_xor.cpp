#include "attacks_xor.h"
#include <array>
#include <range/v3/all.hpp>
#include <vector>

using pals::bytestr::ByteStr;
namespace R = ranges::views;

namespace pals::attacks_xor {

std::tuple<uint8_t, double> solve_xor1(const ByteStr &bs) {
    std::vector<std::tuple<int, double>> phase1 = utils::best_elements(
        R::iota(0, 256),
        [&](const auto &x) { return scoring::LetterUnigramAnalyzer::score(bs ^ x); },
        std::greater{},
        5);

    const auto &[best_key, best_score] = utils::best_element(
        phase1 | R::transform([](const auto &p) { return std::get<0>(p); }),
        [&](const auto &x) { return scoring::score1(bs ^ x); },
        std::greater{});

    return {static_cast<uint8_t>(best_key), best_score};
}

std::tuple<uint8_t, double> solve_xor1_simple(const ByteStr &bs) {
    const auto &[best_key, best_score] = utils::best_element(
        R::iota(0, 256),
        [&](const auto &x) { return scoring::LetterUnigramAnalyzer::score(bs ^ x); },
        std::greater{});

    return {static_cast<uint8_t>(best_key), best_score};
}


} // namespace pals::attacks_xor
