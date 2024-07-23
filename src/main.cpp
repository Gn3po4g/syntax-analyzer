#include <print>
#include <stdexcept>

#include "grammar.hpp"

auto main() -> int {
  try {
    analyzer::parse_grammar("grammars.txt");
    analyzer::parse_token("tokens.txt");
    analyzer::init_first();
    analyzer::init_follow();
    analyzer::init_table();
    analyzer::analyse();
    std::println("Analysis completed.");
  } catch (const std::invalid_argument &e) {
    std::println("{}", e.what());
  }
  return 0;
}