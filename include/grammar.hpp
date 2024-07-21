#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <array>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

struct grammar_t {
  std::string left;
  std::vector<std::string> right;
};

struct token_t {
  std::string value;
  std::string type;
};

using ff_t    = std::map<std::string, std::set<std::string>>;
using table_t = std::map<std::string, std::map<std::string, std::vector<std::string>>>;

constexpr std::string EMPTY = "$";
constexpr std::string END   = "#";
constexpr std::array<std::string, 26> terminal{EMPTY,  "const",  ",", ";",   "Ident", "=",  "int", "(", ")",
                                               "void", "{",      "}", "INT", "+",     "-",  "!",   "*", "/",
                                               "%",    "return", "<", ">",   "<=",    ">=", "==",  "!="};

auto parse_token(std::string_view) -> std::vector<token_t>;

class Grammar {
public:
  explicit Grammar(std::string_view);
  void Analyse(const std::vector<token_t> &);

private:
  void ReadGrammar(std::string_view);
  void InitFirst();
  void InitFollow();
  void InitTable();

  std::vector<grammar_t> grammars;
  ff_t first;
  ff_t follow;
  table_t table;
};

#endif