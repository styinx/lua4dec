#include "ast/ast.hpp"
#include "lua/lua.hpp"

struct Token
{
    Instruction instruction;
    Function*   function;
};

using TokenList   = std::vector<Token>;
using Action      = void (*)(Ast*&, const Token&);
using ActionTable = std::unordered_map<Operator, Action>;

TokenList parse_bytecode(const Function&);
void      run_parser_machine(Ast*& ast, const TokenList&);