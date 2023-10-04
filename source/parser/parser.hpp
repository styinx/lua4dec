#include "ast/ast.hpp"
#include "lua/lua.hpp"

using Action      = void (*)(Ast*&, const Instruction&, const Function&);
using ActionTable = std::unordered_map<Operator, Action>;

void parse_function(Ast*& ast, const Function&);
