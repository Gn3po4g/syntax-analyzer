#include "grammar.hpp"

#include <fstream>
#include <ios>
#include <iterator>
#include <numeric>
#include <ranges>
#include <regex>
#include <string_view>

using namespace std;

const auto ff_size = [](const ff_t &map) {
  return accumulate(map.begin(), map.end(), 0uz, [](size_t init, const ff_t::value_type &entry) {
    const auto &[l, r] = entry;
    return init + r.size();
  });
};

Grammar::Grammar(const string_view grammar_file) {
  ReadGrammar(grammar_file);
  InitFirst();
  InitFollow();
  InitTable();
}

void Grammar::ReadGrammar(const string_view file_name) {
  ifstream file(file_name.data());
  if (!file.is_open()) { throw ios_base::failure("Grammar file not found."); }

  for (string line; getline(file, line);) {
    if (smatch m; regex_match(line, m, regex("^(.+) -> (.+)$")) && m.size() == 3) {
      const auto l = m[1].str();
      const auto r = m[2].str();
      const auto v = r | views::split(' ') | views::transform([](auto word) { return string(string_view(word)); })
                   | ranges::to<vector>();
      grammars.emplace_back(l, v);
    }
  }
}

void Grammar::InitFirst() {
  for (const auto &item : vt) first[item].insert(item);
  size_t cur_size = 0;
  do {
    cur_size = ff_size(first);
    for (const auto &[l, r] : grammars) {
      auto it = r.begin();
      for (; it != r.end(); it++) {
        if (vt.contains(*it)) {
          first[l].insert(*it);
          break;
        } else {
          auto set = first[*it];
          if (set.contains("$")) {
            set.erase("$");
            for (const auto &item : set) first[l].insert(item);
          } else {
            for (const auto &item : set) first[l].insert(item);
            break;
          }
        }
      }
      if (it == r.end()) first[l].insert("$");
    }
  } while (cur_size < ff_size(first));
}

void Grammar::InitFollow() {
  follow[grammars[0].first].insert("#");
  size_t cur_size = 0;
  do {
    cur_size = ff_size(follow);
    for (const auto &[l, r] : grammars) {
      for (const auto &item : follow[l]) follow[r.back()].insert(item);
      for (auto it = r.begin(); it + 1 != r.end(); it++) {
        auto &set = first[*(it + 1)];
        set.erase("$");
        for (const auto &item : set) follow[*it].insert(item);
      }
      for (auto it = r.rbegin(); it + 1 != r.rend(); it++) {
        if (!first[*it].contains("$")) break;
        else
          for (const auto &item : follow[l]) follow[*(it + 1)].insert(item);
      }
    }
  } while (cur_size < ff_size(follow));
}

void Grammar::InitTable() {
  for (const auto &[l, r] : grammars) {
    for (auto it = r.begin(); it != r.end(); it++) {
      auto &set = first[*it];
      if (!set.contains("$")) {
        for (const auto &item : set) table[l][item] = r;
        break;
      } else {
        set.erase("$");
        for (const auto &item : set) table[l][item] = r;
      }
    }
  }
}

void Grammar::ReadToken(const std::string_view file_name) {
  fstream file(file_name.data());
  if (!file.is_open()) { throw ios_base::failure("Token file not found."); }

  for (string line; getline(file, line);) {
    if (smatch m; regex_match(line, m, regex("^(.+)\t<(.+),.+>$")) && m.size() == 3) {
      const auto s1 = m[1].str();
      const auto s2 = m[2].str() == "Ident" || m[2].str() == "INT" ? m[2].str() : m[1].str();
      tokens.emplace_back(s1, s2);
    }
  }
  tokens.emplace_back("EOF", "EOF");
}

void Grammar::Analyse(const string_view token_file) {
  stack<string> symStack;
  ReadToken(token_file);
  symStack.push("EOF");
  symStack.push(grammars[0].first);
  int seq = 0;
  ofstream outfile("output.txt");
  for (auto pToken = tokens.begin(); pToken != tokens.end();) {
    outfile << ++seq << '\t' << symStack.top() << '#' << pToken->first << '\t';
    if (symStack.top() == pToken->second) {
      if (symStack.top() == "EOF") {
        outfile << "accept\n";
        break;
      } else {
        symStack.pop();
        pToken++;
        outfile << "move\n";
      }
    } else {
      auto v = table[symStack.top()][pToken->second];
      symStack.pop();
      for (auto it = v.rbegin(); it != v.rend(); it++) { symStack.push(*it); }
      outfile << "reduction\n";
    }
  }
}