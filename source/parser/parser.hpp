#ifndef LUA4DEC_PARSER_H
#define LUA4DEC_PARSER_H

#include "ast/ast.hpp"
#include "errors.hpp"

/*
 * Remembering the state of a closure. Every closure needs their own stack and PC.
 * Local offsets depend on the scope which has to be kept track of.
 */
struct State
{
    unsigned           PC = 0;
    unsigned           local_offset = 0;
    unsigned           scope_level = 0;
    Vector<AstElement> stack;
};

/*
 * Every operator and therefore instruction, is mapped to a parsing function.
 * Each parsing function is passed the current state, the current program as AST, the
 * instruction, and the lua function that is parsed.
 * The parsing function returns a value > 0 if an errors occurred.
 */
using Action      = Error (*)(State& state, Ast*&, const Instruction&, const Function&);
using ActionTable = std::unordered_map<Operator, Action>;

Error parse_function(State&, Ast*&, const Function&);

#endif  // LUA4DEC_PARSER_H
