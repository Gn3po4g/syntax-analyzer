#include "grammar.hpp"
#include <ios>
#include <print>

auto main() -> int {
  try {
    const auto tokens = parse_token("tokens.txt");
    Grammar("grammars.txt").Analyse(tokens);
    std::println("Analysis completed.");
  } catch (const std::ios_base::failure &e) {
    std::println(stderr, "{}", e.what());
    return 1;
  }
  return 0;
}