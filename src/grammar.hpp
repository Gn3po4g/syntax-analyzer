#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <set>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

using std::pair;
using std::set;
using std::stack;
using std::string;
using std::string_view;
using std::unordered_map;
using std::vector;
using grammar_t = vector<pair<string, vector<string>>>;
using token_t = vector<pair<string, string>>;
using ff_t = unordered_map<string, set<string>>;
using table_t = unordered_map<string, unordered_map<string, vector<string>>>;

class Grammar {
public:
  Grammar(const string_view);
  void Analyse(const string_view);

private:
  void ReadGrammar(const string_view);
  void ReadToken(const string_view);
  void InitFirst();
  void InitFollow();
  void InitTable();

  grammar_t grammars;
  token_t tokens;
  ff_t first;
  ff_t follow;
  table_t table;
  set<string> vn;
  set<string> vt{"$",    "const",  ",", ";",   "Ident", "=",  "int", "(", ")",
                 "void", "{",      "}", "INT", "+",     "-",  "!",   "*", "/",
                 "%",    "return", "<", ">",   "<=",    ">=", "==",  "!="};
};

#endif