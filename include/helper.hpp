#ifndef _ANALYZER_HELPER_H_
#define _ANALYZER_HELPER_H_

#include <format>
#include <string>
#include <string_view>
#include <vector>

template <typename T>
struct std::formatter<std::vector<T>> : std::formatter<std::string_view> {
  constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

  auto format(const std::vector<T>& vec, std::format_context& ctx) const {
    std::string res;
    for (const auto& elem : vec) {
      std::format_to(std::back_inserter(res), "{} ", elem);
    }
    res.pop_back();
    return std::formatter<std::string_view>::format(res, ctx);
  }
};

#endif