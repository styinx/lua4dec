#ifndef LUA4DEC_PARSER_H
#define LUA4DEC_PARSER_H

#include "ast/ast.hpp"

using Stack = Vector<AstElement>;

struct State
{
    Stack    stack;
    unsigned PC = 0;
};

using Action      = void (*)(State& state, Ast*&, const Instruction&, const Function&);
using ActionTable = std::unordered_map<Operator, Action>;

void parse_function(State&, Ast*&, const Function&);

#endif  // LUA4DEC_PARSER_H
