#include "grammar.hpp"

#include <algorithm>
#include <fstream>
#include <ios>
#include <iterator>
#include <ranges>
#include <regex>
#include <string_view>

using std::string;
using std::string_view;

auto parse_token(const std::string_view file_name) -> std::vector<token_t> {
  std::vector<token_t> res;
  std::fstream file(file_name.data());
  if (!file.is_open()) { throw std::ios_base::failure("Token file not found."); }

  for (string line; getline(file, line);) {
    if (std::smatch match; regex_match(line, match, std::regex("^(.+)\t<(.+),.+>$")) && match.size() == 3) {
      const auto match1 = match[1].str();
      const auto match2 = match[2].str();
      res.emplace_back(match1, match2 == "Ident" || match2 == "INT" ? match2 : match1);
    }
  }
  res.emplace_back(END, END);
  file.close();
  return res;
}

Grammar::Grammar(const string_view grammar_file) {
  ReadGrammar(grammar_file);
  InitFirst();
  InitFollow();
  InitTable();
}

void Grammar::ReadGrammar(const string_view file_name) {
  std::ifstream file(file_name.data());
  if (!file.is_open()) { throw std::ios_base::failure("Grammar file not found."); }

  for (string line; getline(file, line);) {
    if (std::smatch match; regex_match(line, match, std::regex("^(.+) -> (.+)$")) && match.size() == 3) {
      const auto left    = match[1].str();
      const auto right   = match[2].str();
      const auto symbols = right | std::views::split(' ')
                         | std::views::transform([](auto word) { return string(string_view(word)); })
                         | std::ranges::to<std::vector>();
      grammars.emplace_back(left, symbols);
    }
  }
}

auto insert_set(std::set<string> &set, auto &&items) -> bool {
  bool changed = false;
  for (const auto &item : items) {
    if (!set.contains(item)) {
      changed = true;
      set.insert(item);
    }
  }
  return changed;
}

struct non_empty {
  friend auto operator|(const std::set<std::string> &set, const non_empty & /*unused*/) {
    return set | std::views::filter([](string_view sym) { return sym != EMPTY; });
  }
};

auto get_range_first(ff_t &table, auto &&symbols) -> std::set<std::string> {
  std::set<std::string> res{EMPTY};
  for (auto &symbol : symbols) {
    if (res.contains(EMPTY)) {
      res.erase(EMPTY);
      res.insert_range(table[symbol]);
    } else {
      break;
    }
  }
  return res;
}

void Grammar::InitFirst() {
  // Init first set
  for (const auto &item : terminal) { first[item].insert(item); }

  bool changed = true;
  while (changed) {
    changed = false;
    std::ranges::for_each(grammars, [this, &changed](const grammar_t &production) {
      const auto &[left, right] = production;

      const auto set = get_range_first(first, right);

      changed = changed || insert_set(first[left], set);
    });
  }
}

void Grammar::InitFollow() {
  follow[grammars[0].left].insert(END);
  bool changed = true;
  while (changed) {
    changed = false;

    std::ranges::for_each(grammars, [this, &changed](const auto &production) {
      const auto &[left, right] = production;

      changed = changed || insert_set(follow[right.back()], follow[left]);
      for (auto it = right.begin(); std::ranges::next(it) != right.end(); std::ranges::advance(it, 1)) {
        const auto range_first = get_range_first(first, std::ranges::subrange(std::ranges::next(it), right.end()));

        changed = changed || insert_set(follow[*it], range_first | non_empty{});

        if (range_first.contains(EMPTY)) { changed = changed || insert_set(follow[*it], follow[left]); }
      }
    });
  }
}

void Grammar::InitTable() {
  std::ranges::for_each(grammars, [this](const auto &production) {
    const auto &[left, right] = production;

    const auto range_first = get_range_first(first, right);
    for (const auto &item : range_first) { table[left][item] = right; }
    if (range_first.contains(EMPTY)) {
      for (const auto &item : follow[left]) { table[left][item] = right; }
    }
  });
}

void Grammar::Analyse(const std::vector<token_t> &tokens) {
  std::stack<string> symbol_stack;
  symbol_stack.emplace(END);
  symbol_stack.push(grammars[0].left);
  int seq = 0;
  std::ofstream outfile("output.txt");
  for (auto token = tokens.begin(); token != tokens.end();) {
    outfile << std::format("{:<5}[top: {:<15}] [current: {:<10}]  ", ++seq, symbol_stack.top(), token->value);
    if (symbol_stack.top() == token->type) {
      outfile << (symbol_stack.top() == END ? "accept\n" : "move\n");
      symbol_stack.pop();
      std::ranges::advance(token, 1);
    } else {
      if (symbol_stack.top() == EMPTY) {
        symbol_stack.pop();
      } else {
        const auto &symbols = table.at(symbol_stack.top()).at(token->type);
        symbol_stack.pop();
        for (const auto &item : std::ranges::reverse_view(symbols)) { symbol_stack.push(item); }
      }
      outfile << "reduction\n";
    }
  }
}