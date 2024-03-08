#ifndef LUA4DEC_PARSER_H
#define LUA4DEC_PARSER_H

#include "ast/ast.hpp"

/*
 * Remembering the state of a closure. Every closure needs their own state.
 */
struct State
{
    unsigned           PC = 0;
    Vector<AstElement> stack;
};

using Action      = void (*)(State& state, Ast*&, const Instruction&, const Function&);
using ActionTable = std::unordered_map<Operator, Action>;

void parse_function(State&, Ast*&, const Function&);

#endif  // LUA4DEC_PARSER_H
