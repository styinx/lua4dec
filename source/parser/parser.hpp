#ifndef LUA4DEC_PARSER_H
#define LUA4DEC_PARSER_H

#include "ast/ast.hpp"

using Action      = void (*)(Ast*&, const Instruction&, const Function&);
using ActionTable = std::unordered_map<Operator, Action>;

void parse_function(Ast*& ast, const Function&);

#endif  // LUA4DEC_PARSER_H
