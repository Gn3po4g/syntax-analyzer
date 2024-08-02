#ifndef _ANALYZER_HELPER_H_
#define _ANALYZER_HELPER_H_

#include <format>
#include <ranges>
#include <vector>

namespace std {
template <typename T>
struct formatter<vector<T>> : formatter<T> {
  using base = formatter<T>;

  auto format(const vector<T>& vec, format_context& ctx) const {
    if (vec.empty()) {
      return format_to(ctx.out(), "");
    }

    for (const auto& elem : vec | views::take(vec.size() - 1)) {
      base::format(elem, ctx);
      format_to(ctx.out(), " ");
    }
    return base::format(vec.back(), ctx);
  }
};

}  // namespace std

#endif