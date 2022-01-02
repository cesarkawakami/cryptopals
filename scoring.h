#pragma once
#include "ByteStr.h"
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <range/v3/all.hpp>
#include <vector>

namespace pals::scoring {

struct LetterUnigramEvtGen {
    const static std::size_t num_events = 26 + 1;
    void operator()(const pals::bytestr::ByteStr &bs, auto rcv) {
        for (uint8_t uch : bs) {
            char c = std::tolower(static_cast<char>(uch));
            if ('a' <= c && c <= 'z') {
                rcv(c - 'a' + 1);
            } else {
                rcv(0);
            }
        }
    }
};

struct AsciiUnigramEvtGen {
    const static std::size_t num_events = 127 - 32 + 1;
    void operator()(const pals::bytestr::ByteStr &bs, auto rcv) {
        char last_char{};
        for (uint8_t uch : bs) {
            char c = static_cast<char>(uch);
            if ('!' <= c && c <= '~') {
                rcv(c - ' ' + 1);
            } else if (std::isspace(c) && !std::isspace(last_char)) {
                rcv(1);
            } else {
                rcv(0);
            }
            last_char = c;
        }
    }
};

const pals::bytestr::ByteStr &english_web_sentences();

template <std::size_t N>
double hellinger(const std::array<double, N> &a, const std::array<double, N> &b) {
    double coeff = 0;
    for (std::size_t i = 0; i < N; ++i) {
        coeff += std::sqrt(a[i] * b[i]);
    }
    return std::sqrt(1 - coeff);
}

template <std::size_t N>
double similarity(const std::array<double, N> &a, const std::array<double, N> &b) {
    return 1 - hellinger(a, b);
}

template <class T>
struct EvtAnalyzer {
    using FreqDist = std::array<double, T::num_events>;

    static FreqDist analyze(const pals::bytestr::ByteStr &bs) {
        FreqDist freq_dist{};
        T gen{};
        double total = 0;
        gen(bs, [&](int i) {
            ++total;
            ++freq_dist[i];
        });
        utils::assume(total > 0, "empty distribution");
        for (double &x : freq_dist) {
            x /= total;
        }
        return freq_dist;
    }
    const static FreqDist &eng_canon() {
        static FreqDist instance{};
        static bool initialized = false;
        if (!initialized) {
            instance = analyze(english_web_sentences());
            initialized = true;
        }
        return instance;
    }
    static double score(const pals::bytestr::ByteStr &bs) {
        const FreqDist &candidate = analyze(bs);
        const FreqDist &expected = eng_canon();
        return similarity(candidate, expected);
    }
};

using LetterUnigramAnalyzer = EvtAnalyzer<LetterUnigramEvtGen>;
using AsciiUnigramAnalyzer = EvtAnalyzer<AsciiUnigramEvtGen>;

double score1(const pals::bytestr::ByteStr &bs);

} // namespace pals::scoring
