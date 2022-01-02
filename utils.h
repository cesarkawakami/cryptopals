#pragma once
#include <fstream>
#include <iostream>
#include <range/v3/all.hpp>
#include <sstream>
#include <string>
#include <type_traits>

namespace pals::utils {

[[noreturn]] void fatal(const std::string &s);

void expect(bool condition, const std::string &msg);

std::string read_file(const std::string &path);

template <class Rng, class Fn, class Cmp>
std::vector<std::tuple<typename ranges::range_value_t<Rng>,
                       typename std::invoke_result_t<Fn, typename ranges::range_value_t<Rng>>>>
best_elements(Rng rng, Fn fn, Cmp cmp, std::size_t limit) {
    using ElemType = ranges::range_value_t<Rng>;
    using ScoreType = std::invoke_result_t<Fn, ElemType>;
    static_assert(sizeof(std::invoke_result_t<Cmp, ScoreType, ScoreType>));
    decltype(best_elements(rng, fn, cmp, limit)) rv;
    rv.reserve(limit + 1);
    for (const auto &el : rng) {
        const auto &score{fn(el)};
        rv.push_back({el, score});
        for (std::size_t i = rv.size(); i >= 2; --i) {
            const auto &[left_el, left_score] = rv[i - 2];
            const auto &[right_el, right_score] = rv[i - 1];
            if (cmp(right_score, left_score)) {
                std::swap(rv[i - 2], rv[i - 1]);
            }
        }
        if (rv.size() > limit) {
            rv.pop_back();
        }
    }
    return rv;
}

template <class Rng, class Fn, class Cmp>
std::tuple<typename ranges::range_value_t<Rng>,
           typename std::invoke_result_t<Fn, typename ranges::range_value_t<Rng>>>
best_element(Rng rng, Fn fn, Cmp cmp) {
    return best_elements(rng, fn, cmp, 1)[0];
}

} // namespace pals::utils
