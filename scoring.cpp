#include "scoring.h"
#include "utils.h"
#include <cmath>
#include <fstream>

using pals::bytestr::ByteStr;

namespace pals::scoring {

const ByteStr &english_web_sentences() {
    static ByteStr singleton{};
    static bool initialized = false;
    if (!initialized) {
        std::ifstream fin{"eng_web_2012_300K-sentences.txt"};
        std::string line;
        while (std::getline(fin, line), fin) {
            auto tab_pos = line.find('\t');
            if (tab_pos == std::string::npos) {
                continue;
            }
            for (auto i = line.size() * 0 + tab_pos + 1; i < line.size(); ++i) {
                singleton.data.push_back(static_cast<uint8_t>(line[i]));
            }
            singleton.data.push_back(static_cast<uint8_t>('\n'));
        }
        initialized = true;
    }
    return singleton;
}

double score1(const pals::bytestr::ByteStr &bs) {
    double letter_score = LetterUnigramAnalyzer::score(bs);
    double ascii_score = AsciiUnigramAnalyzer::score(bs);
    return (9 * letter_score + 1 * ascii_score) / 10;
}

} // namespace pals::scoring
