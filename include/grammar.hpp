#ifndef _ANALYZER_GRAMMAR_H_
#define _ANALYZER_GRAMMAR_H_

#include <array>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace analyzer {

using std::map;
using std::set;
using std::string;
using std::string_view;
using std::vector;

using sequence = vector<string>;

struct grammar_t {
  string left;
  sequence right;
};

struct token_t {
  string value;
  string type;
};

using ff_t = map<string, set<string>>;
using table_t = map<string, map<string, sequence>>;

constexpr string EMPTY = "$";
constexpr string END = "#";
constexpr std::array<string, 25> terminals{
    "const",  ",", ";",   "Ident", "=",  "int", "(", ")", "void",
    "{",      "}", "INT", "+",     "-",  "!",   "*", "/", "%",
    "return", "<", ">",   "<=",    ">=", "==",  "!="};

void parse_grammar(string_view);
void parse_token(string_view);
void init_first() noexcept;
void init_follow() noexcept;
void init_table() noexcept;
void analyse() noexcept;

}  // namespace analyzer

#endif