#include "grammar.hpp"

#include <algorithm>
#include <fstream>
#include <print>
#include <ranges>
#include <regex>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>

#include "helper.hpp"  // IWYU pragma: keep

namespace analyzer {
vector<grammar_t> grammars;
vector<token_t> tokens;
ff_t first;
ff_t follow;
table_t table;
set non_terminals;
string start;

void parse_token(const string_view file_name) {
  std::fstream file(file_name.data());
  if (!file.is_open()) {
    throw std::invalid_argument("Token file not found.");
  }

  for (string line; getline(file, line);) {
    if (std::smatch match;
        regex_match(line, match, std::regex("^(.+)\t<(.+),.+>$")) &&
        match.size() == 3) {
      const auto match1 = match[1].str();
      const auto match2 = match[2].str();
      tokens.emplace_back(
        match1, match2 == "Ident" || match2 == "INT" ? match2 : match1);
    }
  }
  tokens.emplace_back(END, END);
  file.close();
}

void parse_grammar(const string_view file_name) {
  std::ifstream file(file_name.data());
  if (!file.is_open()) {
    throw std::invalid_argument("Grammar file not found.");
  }

  bool first_line = true;
  for (string line; getline(file, line); first_line = false) {
    if (std::smatch match;
        std::regex_match(line, match, std::regex("^(.+) -> (.+)$")) &&
        match.size() == 3) {
      const auto left = match[1].str();
      non_terminals.insert(left);
      if (first_line) {
        start = left;
      }
      const auto right = match[2].str();
      const auto symbols =
        right | std::views::split(' ') | std::ranges::to<phrase>();
      grammars.emplace_back(left, symbols);
    }
  }
}

auto changed_after_insert(set &set, auto &&items) -> bool {
  bool changed = false;
  for (const auto &item : items) {
    if (!set.contains(item)) {
      changed = true;
      set.insert(item);
    }
  }
  return changed;
}

auto get_range_first(ff_t &table, auto &&symbols) -> set {
  set res{EMPTY};
  if (!std::ranges::equal(res, symbols)) {
    for (const auto &symbol : symbols) {
      if (res.contains(EMPTY)) {
        res.erase(EMPTY);
        const auto &set = table.at(symbol);
        res.insert(set.begin(), set.end());
      } else {
        break;
      }
    }
  }
  return res;
}

void init_first() noexcept {
  for (const auto &item : terminals) {
    first.emplace(item, set{item});
  }
  for (const auto &item : non_terminals) {
    first.emplace(item, set{});
  }

  bool changed = true;
  while (changed) {
    changed = false;
    std::ranges::for_each(grammars, [&changed](const grammar_t &production) {
      const auto &[left, right] = production;

      const auto set = get_range_first(first, right);

      changed |= changed_after_insert(first.at(left), set);
    });
  }
}

const struct not_empty_s {
  auto operator()(string_view str) { return str != EMPTY; }
} not_empty;

void init_follow() noexcept {
  for (const auto &item : non_terminals) {
    if (item == start) {
      follow.emplace(item, set{END});
    } else {
      follow.emplace(item, set{});
    }
  }

  bool changed = true;
  while (changed) {
    changed = false;

    std::ranges::for_each(grammars, [&changed](const grammar_t &production) {
      const auto &[left, right] = production;

      // changed |= changed_after_insert(follow.at(right.back()),
      // follow.at(left));
      for (auto it = right.begin(); it != right.end();
           std::ranges::advance(it, 1)) {
        if (!non_terminals.contains(*it)) {
          continue;
        }
        const auto range_first = get_range_first(
          first, std::ranges::subrange(std::ranges::next(it), right.end()));

        changed |= changed_after_insert(
          follow.at(*it), range_first | std::views::filter(not_empty));

        if (range_first.contains(EMPTY) ||
            std::ranges::next(it) == right.end()) {
          changed |= changed_after_insert(follow.at(*it), follow.at(left));
        }
      }
    });
  }
}

void init_table() noexcept {
  for (const auto &non_term : non_terminals) {
    table.emplace(non_term, map<string, phrase>{});
  }
  std::ranges::for_each(grammars, [](const auto &production) {
    const auto &[left, right] = production;

    const auto range_first = get_range_first(first, right);

    for (const auto &item : range_first | std::views::filter(not_empty)) {
      table.at(left).emplace(item, right);
    }

    if (range_first.contains(EMPTY)) {
      for (const auto &item : follow.at(left)) {
        table.at(left).emplace(item, right);
      }
    }
  });
}

void analyse() noexcept {
  std::stack<string> symbol_stack;
  symbol_stack.emplace(END);
  symbol_stack.push(start);

  int seq = 0;

  std::ofstream outfile("output.txt");

  for (auto token = tokens.begin(); token != tokens.end();) {
    std::print(outfile, "{:<5}[top: {:<15}] [current: {:<10}]  ", ++seq,
               symbol_stack.top(), token->value);

    if (symbol_stack.top() == token->type) {
      std::println(outfile, "{}",
                   symbol_stack.top() == END ? "accept" : "move");

      symbol_stack.pop();

      std::ranges::advance(token, 1);
    } else {
      const auto &symbols = table.at(symbol_stack.top()).at(token->type);

      std::println(outfile, "{} -> {}", symbol_stack.top(), symbols);

      symbol_stack.pop();

      for (const auto &item :
           std::ranges::reverse_view(symbols) | std::views::filter(not_empty)) {
        symbol_stack.push(item);
      }
    }
  }
}
}  // namespace analyzer