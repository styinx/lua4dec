#include "parser/parser.hpp"

#include <algorithm>

// Get a local variable that is defined at the current PC offset.
// Use the index to get the nth next variable.
String get_local_from_pc(const Function& function, unsigned pc_offset, unsigned index = 0)
{
    for(unsigned i = 0; i < function.locals.size(); ++i)
    {
        if(function.locals[i].start_pc >= pc_offset)
            if(i + index < function.locals.size())
                return function.locals[i + index].name;
    }
    return "";
}

Error enter_block(State&, Ast*&, const Instruction&, const Function&);
Error exit_block(State&, Ast*&);

Error handle_end(State&, Ast*&, const Instruction&, const Function&);
Error handle_return(State&, Ast*&, const Instruction&, const Function&);
Error handle_call(State&, Ast*&, const Instruction&, const Function&);
Error handle_tail_call(State&, Ast*&, const Instruction&, const Function&);
Error handle_push_nil(State&, Ast*&, const Instruction&, const Function&);
Error handle_pop(State&, Ast*&, const Instruction&, const Function&);
Error handle_push_int(State&, Ast*&, const Instruction&, const Function&);
Error handle_push_string(State&, Ast*&, const Instruction&, const Function&);
Error handle_push_num(State&, Ast*&, const Instruction&, const Function&);
Error handle_push_neg_num(State&, Ast*&, const Instruction&, const Function&);
Error handle_push_upvalue(State&, Ast*&, const Instruction&, const Function&);
Error handle_get_local(State&, Ast*&, const Instruction&, const Function&);
Error handle_get_global(State&, Ast*&, const Instruction&, const Function&);
Error handle_get_table(State&, Ast*&, const Instruction&, const Function&);
Error handle_get_dotted(State&, Ast*&, const Instruction&, const Function&);
Error handle_get_indexed(State&, Ast*&, const Instruction&, const Function&);
Error handle_push_self(State&, Ast*&, const Instruction&, const Function&);
Error handle_create_table(State&, Ast*&, const Instruction&, const Function&);
Error handle_set_local(State&, Ast*&, const Instruction&, const Function&);
Error handle_set_global(State&, Ast*&, const Instruction&, const Function&);
Error handle_set_table(State&, Ast*&, const Instruction&, const Function&);
Error handle_set_list(State&, Ast*&, const Instruction&, const Function&);
Error handle_set_map(State&, Ast*&, const Instruction&, const Function&);
Error handle_add(State&, Ast*&, const Instruction&, const Function&);
Error handle_addi(State&, Ast*&, const Instruction&, const Function&);
Error handle_sub(State&, Ast*&, const Instruction&, const Function&);
Error handle_mult(State&, Ast*&, const Instruction&, const Function&);
Error handle_div(State&, Ast*&, const Instruction&, const Function&);
Error handle_pow(State&, Ast*&, const Instruction&, const Function&);
Error handle_concat(State&, Ast*&, const Instruction&, const Function&);
Error handle_minus(State&, Ast*&, const Instruction&, const Function&);
Error handle_not(State&, Ast*&, const Instruction&, const Function&);
Error handle_jmpont(State&, Ast*&, const Instruction&, const Function&);
Error handle_jmp(State&, Ast*&, const Instruction&, const Function&);
Error handle_push_niljump(State&, Ast*&, const Instruction&, const Function&);
Error handle_forprep(State&, Ast*&, const Instruction&, const Function&);
Error handle_lforprep(State&, Ast*&, const Instruction&, const Function&);
Error handle_forloop(State&, Ast*&, const Instruction&, const Function&);
Error handle_lforloop(State&, Ast*&, const Instruction&, const Function&);
Error handle_closure(State&, Ast*&, const Instruction&, const Function&);

Error handle_condition(State&, Ast*&, const Instruction&, const Function&);

// clang-format off
auto TABLE = ActionTable
{
    {Operator::END,         &handle_end},
    {Operator::RETURN,      &handle_return},
    {Operator::CALL,        &handle_call},
    {Operator::TAILCALL,    &handle_tail_call},
    {Operator::PUSHNIL,     &handle_push_nil},
    {Operator::POP,         &handle_pop},
    {Operator::PUSHINT,     &handle_push_int},
    {Operator::PUSHSTRING,  &handle_push_string},
    {Operator::PUSHNUM,     &handle_push_num},
    {Operator::PUSHNEGNUM,  &handle_push_neg_num},
    {Operator::PUSHUPVALUE, &handle_push_upvalue}, // TODO
    {Operator::GETLOCAL,    &handle_get_local},
    {Operator::GETGLOBAL,   &handle_get_global},
    {Operator::GETTABLE,    &handle_get_table},
    {Operator::GETDOTTED,   &handle_get_dotted},
    {Operator::GETINDEXED,  &handle_get_indexed},
    {Operator::PUSHSELF,    &handle_push_self},
    {Operator::CREATETABLE, &handle_create_table},
    {Operator::SETLOCAL,    &handle_set_local},
    {Operator::SETGLOBAL,   &handle_set_global},
    {Operator::SETTABLE,    &handle_set_table},
    {Operator::SETLIST,     &handle_set_list},
    {Operator::SETMAP,      &handle_set_map},
    {Operator::ADD,         &handle_add},
    {Operator::ADDI,        &handle_addi},
    {Operator::SUB,         &handle_sub},
    {Operator::MULT,        &handle_mult},
    {Operator::DIV,         &handle_div},
    {Operator::POW,         &handle_pow},
    {Operator::CONCAT,      &handle_concat},
    {Operator::MINUS,       &handle_minus},
    {Operator::NOT,         &handle_not},
    {Operator::JMPNE,       &handle_condition},
    {Operator::JMPEQ,       &handle_condition},
    {Operator::JMPLT,       &handle_condition},
    {Operator::JMPLE,       &handle_condition},
    {Operator::JMPGT,       &handle_condition},
    {Operator::JMPGE,       &handle_condition},
    {Operator::JMPT,        &handle_condition},
    {Operator::JMPF,        &handle_condition},
    {Operator::JMPONT,      &handle_jmpont},
    {Operator::JMPONF,      &handle_condition},
    {Operator::JMP,         &handle_jmp},
    {Operator::PUSHNILJMP,  &handle_push_niljump},
    {Operator::FORPREP,     &handle_forprep},
    {Operator::FORLOOP,     &handle_forloop},
    {Operator::LFORPREP,    &handle_lforprep},
    {Operator::LFORLOOP,    &handle_lforloop},
    {Operator::CLOSURE,     &handle_closure},
};

// clang-format on

/*
 * Helper functions
 */

Error enter_block(Ast*& ast)
{
    auto* child   = new Ast();
    child->parent = ast;

    ast->child = child;
    ast        = child;

    return Error::NONE;
}

Error exit_block(Ast*& ast)
{
    if(ast->parent)
        ast = ast->parent;

    return Error::NONE;
}

/*
 * Parsing functions
 */

/*
 * Arguments:       -
 * Stack before:    -
 * Stack after:     -
 * Side effects:    -
 *
 * @brief   Operator has no effect and is handled implicitly by the other operators.
 */
Error handle_end(State&, Ast*&, const Instruction&, const Function&)
{
    return Error::NONE;
}

/*
 * Arguments:       U
 * Stack before:    v_n - v_x (at U)    | meaning v_n is at position 'U'
 * Stack after:     return
 * Side effects:    returns v_x - v_n
 *
 * @brief   Pops elements from the stack until it has a size of 'U'. The popped elements
 *          are returned in reverse order.
 */
Error handle_return(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    auto u = U(instruction);  // U marks the position of the arguments

    Vector<Expression> args;
    while(state.stack.size() > u)
    {
        args.push_back(std::get<Expression>(state.stack.back()));
        state.stack.pop_back();
    }

    std::reverse(args.begin(), args.end());

    Return ret(args);
    ast->statements.push_back(ret);

    return Error::NONE;
}

/*
 * Arguments:       A B
 * Stack before:    v_n - v_1  f (at A)     | meaning that function name is a position 'A'
 * Stack after:     r_b - r_1
 * Side effects:    f(v1, ..., v_n)
 *
 * @brief   Pops elements from the stack until it has a size of 'A'. The last popped
 *          element is the caller. The rest of the popped elements are the arguments
 *          in reversed order.
 *          In case the caller is a table or a map (bot of type AstTable) we just push
 *          it back onto the stack.
 */
Error handle_call(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto a = A(instruction);  // The caller is at position a
    const auto b = B(instruction);  // Is expression (!=0) or statement (0)

    Vector<Expression> args;
    while(state.stack.size() > a + 1)
    {
        args.push_back(std::get<Expression>(state.stack.back()));
        state.stack.pop_back();
    }

    auto caller = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    // Caller is a regular function
    if(std::holds_alternative<Identifier>(caller))
    {
        auto name = std::get<Identifier>(caller);

        std::reverse(args.begin(), args.end());

        if(B(instruction) == 0)
            ast->statements.push_back(Call(name, args));
        else
            state.stack.push_back(Expression(Call(name, args, true)));
    }
    // Caller is a table
    else if(std::holds_alternative<AstTable>(caller))
    {
        auto table = std::get<AstTable>(caller);
        state.stack.push_back(table);
    }
    else
    {
        return Error::BAD_VARIANT;
    }

    return Error::NONE;
}

/*
 * Arguments:       A B
 * Stack before:    v_n - v_1  f (at A)     | meaning that function name is a position 'A'
 * Stack after:     return
 * Side effects:    f(v1, ..., v_n)
 *
 * @brief
 */
Error handle_tail_call(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto a = A(instruction);  // The caller is at position a

    Vector<Expression> args;
    while(state.stack.size() > a + 1)
    {
        args.push_back(std::get<Expression>(state.stack.back()));
        state.stack.pop_back();
    }

    auto name = std::get<Identifier>(std::get<Expression>(state.stack.back()));
    state.stack.pop_back();

    std::reverse(args.begin(), args.end());

    ast->statements.push_back(TailCall(name, args));

    return Error::NONE;
}

/*
 * Arguments:       U
 * Stack before:    -
 * Stack after:     nil_1 - nil_u
 * Side effects:    -
 *
 * @brief
 */
Error handle_push_nil(State& state, Ast*& ast, const Instruction&, const Function&)
{
    // TODO
    state.stack.push_back(Identifier("nil"));

    return Error::NONE;
}

/*
 * Arguments:       U
 * Stack before:    -
 * Stack after:     a_u - a_1
 * Side effects:    -
 *
 * @brief
 */
Error handle_pop(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    auto u = U(instruction);

    while(u > 0)
    {
        if(state.stack.empty())
            return Error::EMPTY_STACK;

        state.stack.pop_back();
        u--;
    }

    return Error::NONE;
}

/*
 * Arguments:       S
 * Stack before:    -
 * Stack after:     (Number)s
 * Side effects:    -
 *
 * @brief
 */
Error handle_push_int(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    const auto s = S(instruction);
    state.stack.push_back(AstInt(s));

    return Error::NONE;
}

/*
 * Arguments:       K       | U used as index for the globals table
 * Stack before:    -
 * Stack after:     KSTR[k]
 * Side effects:    -
 *
 * @brief
 */
Error handle_push_string(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto k      = U(instruction);
    const auto string = function.globals[k];

    state.stack.push_back(AstString(string));

    return Error::NONE;
}

/*
 * Arguments:       N       | U used as index for the number table
 * Stack before:    -
 * Stack after:     KNUM[n]
 * Side effects:    -
 *
 * @brief
 */
Error handle_push_num(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto n      = U(instruction);
    const auto number = function.numbers[n];
    state.stack.push_back(AstNumber(number));

    return Error::NONE;
}

/*
 * Arguments:       N       | U used as index for the number table
 * Stack before:    -
 * Stack after:     -KNUM[n]
 * Side effects:    -
 *
 * @brief
 */
Error handle_push_neg_num(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    return handle_push_num(state, ast, instruction, function);
}

/*
 * Arguments:       U
 * Stack before:    -
 * Stack after:     Closure[u]
 * Side effects:    -
 *
 * @brief
 */
Error handle_push_upvalue(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    return Error::NONE;
}

/*
 * Arguments:       L       | U used as index for the local table
 * Stack before:    -
 * Stack after:     LOC[l]
 * Side effects:    -
 *
 * @brief
 */
Error handle_get_local(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    // TODO: This is a special hack for SWBF
    const auto pos  = U(instruction);
    auto       name = String("local" + std::to_string(pos));
    if(function.locals.size() > pos)
        name = function.locals[pos].name;
    state.stack.push_back(Identifier(name));

    return Error::NONE;
}

/*
 * Arguments:       K       | U used as index for the globals table
 * Stack before:    -
 * Stack after:     VAR[KSTR[k]]
 * Side effects:    -
 *
 * @brief
 */
Error handle_get_global(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto name = function.globals[U(instruction)];
    state.stack.push_back(Identifier(name));

    return Error::NONE;
}

/*
 * Arguments:       -
 * Stack before:    i t
 * Stack after:     t[i]
 * Side effects:    -
 *
 * @brief
 */
Error handle_get_table(State& state, Ast*& ast, const Instruction&, const Function&)
{
    const auto index = std::get<Identifier>(std::get<Expression>(state.stack.back()));
    state.stack.pop_back();

    const auto table = std::get<Identifier>(std::get<Expression>(state.stack.back()));
    state.stack.pop_back();

    state.stack.push_back(Identifier(table.name + "[" + index.name + "]"));

    return Error::NONE;
}

/*
 * Arguments:       K
 * Stack before:    t
 * Stack after:     t[KSTR[k]]
 * Side effects:    -
 *
 * @brief
 */
Error handle_get_dotted(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto name   = function.globals[U(instruction)];
    const auto parent = std::get<Identifier>(std::get<Expression>(state.stack.back()));
    state.stack.pop_back();
    state.stack.push_back(Identifier(parent.name + "." + name));

    return Error::NONE;
}

/*
 * Arguments:       L
 * Stack before:    t
 * Stack after:     t[LOC[l]]
 * Side effects:    -
 *
 * @brief
 */
Error handle_get_indexed(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto name   = function.locals[U(instruction)].name;
    const auto parent = std::get<Identifier>(std::get<Expression>(state.stack.back()));
    state.stack.pop_back();
    state.stack.push_back(Identifier(parent.name + "[" + name + "]"));

    return Error::NONE;
}

/*
 * Arguments:       K
 * Stack before:    t
 * Stack after:     t t[KSTR[k]]
 * Side effects:    -
 *
 * @brief
 */
Error handle_push_self(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto name   = function.globals[U(instruction)];
    const auto parent = std::get<Identifier>(std::get<Expression>(state.stack.back()));
    state.stack.push_back(Identifier(parent.name + "." + name));

    return Error::NONE;
}

/*
 * Arguments:       U
 * Stack before:    -
 * Stack after:     newarray(size = u)
 * Side effects:    -
 *
 * @brief
 */
Error handle_create_table(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    // Its only a table if an identifier is on the stack before. Otherwise its a map or
    // list.

    std::string name;
    if(state.stack.size())
    {
        const auto ex = std::get<Expression>(state.stack.back());
        if(std::holds_alternative<Identifier>(ex))
        {
            name = std::get<Identifier>(ex).name;
            state.stack.pop_back();
        }
    }

    const auto size = U(instruction);
    AstTable   table(size, name, Vector<std::pair<Expression, Expression>>{});
    state.stack.push_back(table);

    return Error::NONE;
}

/*
 * Arguments:       L
 * Stack before:    x
 * Stack after:     -
 * Side effects:    LOC[l] = x
 *
 * @brief
 */
Error handle_set_local(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto left  = Identifier(function.locals[B(instruction)].name);
    const auto right = std::get<Expression>(state.stack.back());
    Assignment ass(left, right);
    state.stack.pop_back();

    ast->statements.push_back(ass);

    return Error::NONE;
}

/*
 * Arguments:       K
 * Stack before:    x
 * Stack after:     -
 * Side effects:    VAR[KSTR[k]] = x
 *
 * @brief
 */
Error handle_set_global(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto left  = Identifier(function.globals[B(instruction)]);
    const auto right = std::get<Expression>(state.stack.back());
    Assignment ass(left, right);
    state.stack.pop_back();

    ast->statements.push_back(ass);

    return Error::NONE;
}

/*
 * Arguments:       A B
 * Stack before:    v a_a - a_1 i t
 * Stack after:     (pops b values)
 * Side effects:    t[i] = v
 *
 * @brief
 */
Error handle_set_table(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto         b = B(instruction);
    Vector<Expression> args;
    for(unsigned i = 0; i < b; ++i)
    {
        args.push_back(std::get<Expression>(state.stack.back()));
        state.stack.pop_back();
    }

    std::reverse(args.begin(), args.end());

    std::string left;
    for(auto it = args.begin(); it != args.end() - 1; ++it)
    {
        if(std::holds_alternative<Identifier>(*it))
            left.append(std::get<Identifier>(*it).name);
        else if(std::holds_alternative<AstString>(*it))
            left.append(std::get<AstString>(*it).value);

        if(it != args.end() - 2)
            left.append(".");
    }

    const auto right = args.back();
    Assignment ass(Identifier(left), right);

    ast->statements.push_back(ass);

    return Error::NONE;
}

/*
 * Arguments:       A B
 * Stack before:    v_b-v_1 t
 * Stack after:     t
 * Side effects:    t[i+a*FPF] = v_i
 *
 * @brief
 */
Error handle_set_list(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    const auto         b = B(instruction);
    Vector<Expression> list;
    for(unsigned i = 0; i < b; ++i)
    {
        list.push_back(std::get<Expression>(state.stack.back()));
        state.stack.pop_back();
    }

    std::reverse(list.begin(), list.end());

    state.stack.pop_back();  // empty AstTable
    state.stack.push_back(AstList(list));

    return Error::NONE;
}

/*
 * Arguments:       U
 * Stack before:    v_u k_u - v_1 k_1 t
 * Stack after:     t
 * Side effects:    t[k_i] = v_i
 *
 * @brief
 */
Error handle_set_map(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    Vector<std::pair<Expression, Expression>> map;
    for(unsigned i = 0; i < U(instruction); ++i)
    {
        const auto value = std::get<Expression>(state.stack.back());
        state.stack.pop_back();

        const auto key = std::get<Expression>(state.stack.back());
        state.stack.pop_back();

        map.push_back(std::pair(key, value));
    }

    std::reverse(map.begin(), map.end());

    auto& table = std::get<AstTable>(std::get<Expression>(state.stack.back()));
    if(table.name.name.empty())
    {
        state.stack.pop_back();  // empty AstTable
        state.stack.push_back(AstMap(map));
    }
    else
    {
        table.pairs = map;
    }

    return Error::NONE;
}

/*
 * Arguments:       -
 * Stack before:    y x
 * Stack after:     x + y
 * Side effects:    -
 *
 * @brief
 */
Error handle_add(State& state, Ast*& ast, const Instruction&, const Function&)
{
    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    state.stack.push_back(AstOperation("+", {left, right}));

    return Error::NONE;
}

/*
 * Arguments:       S
 * Stack before:    x
 * Stack after:     x + s
 * Side effects:    -
 *
 * @brief
 */
Error handle_addi(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    const auto right = AstNumber(S(instruction));
    state.stack.push_back(AstOperation("+", {right, left}));

    return Error::NONE;
}

/*
 * Arguments:       -
 * Stack before:    y x
 * Stack after:     x - y
 * Side effects:    -
 *
 * @brief
 */
Error handle_sub(State& state, Ast*& ast, const Instruction&, const Function&)
{
    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    state.stack.push_back(AstOperation("-", {left, right}));

    return Error::NONE;
}

/*
 * Arguments:       -
 * Stack before:    y x
 * Stack after:     x * y
 * Side effects:    -
 *
 * @brief
 */
Error handle_mult(State& state, Ast*& ast, const Instruction&, const Function&)
{
    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    state.stack.push_back(AstOperation("*", {left, right}));

    return Error::NONE;
}

/*
 * Arguments:       -
 * Stack before:    y x
 * Stack after:     x / y
 * Side effects:    -
 *
 * @brief
 */
Error handle_div(State& state, Ast*& ast, const Instruction&, const Function&)
{
    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    state.stack.push_back(AstOperation("/", {left, right}));

    return Error::NONE;
}

/*
 * Arguments:       -
 * Stack before:    y x
 * Stack after:     x ^ y
 * Side effects:    -
 *
 * @brief
 */
Error handle_pow(State& state, Ast*& ast, const Instruction&, const Function&)
{
    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    state.stack.push_back(AstOperation("^", {left, right}));

    return Error::NONE;
}

/*
 * Arguments:       U
 * Stack before:    v_u - v_1
 * Stack after:     v1 - v_u
 * Side effects:    -
 *
 * @brief
 */
Error handle_concat(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    Vector<Expression> ex;
    for(unsigned i = 0; i < U(instruction); ++i)
    {
        ex.push_back(std::get<Expression>(state.stack.back()));
        state.stack.pop_back();
    }
    state.stack.push_back(AstOperation("..", ex));

    return Error::NONE;
}

/*
 * Arguments:       -
 * Stack before:    x
 * Stack after:     -x
 * Side effects:    -
 *
 * @brief
 */
Error handle_minus(State& state, Ast*& ast, const Instruction&, const Function&)
{
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    state.stack.push_back(AstOperation("-", {right}));

    return Error::NONE;
}

/*
 * Arguments:       -
 * Stack before:    x
 * Stack after:     (x == nil) ? 1 : nil
 * Side effects:    -
 *
 * @brief
 */
Error handle_not(State& state, Ast*& ast, const Instruction&, const Function&)
{
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    state.stack.push_back(AstOperation("not", {right}));

    return Error::NONE;
}

/*
 * Arguments:       -
 * Stack before:    -
 * Stack after:     -
 * Side effects:    -
 *
 * @brief
 */
Error handle_condition(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    const auto op = OP(instruction);

    if(op < Operator::JMPNE || op > Operator::JMPONF)
    {
        printf("Invalid binary operator OP %d (%s) \n", (int)op, OP_TO_STR[op].c_str());
        exit(1);
    }

    std::string comparison;

    switch(op)
    {
    case Operator::JMPNE:
        comparison = "==";
        break;
    case Operator::JMPEQ:
        comparison = "~=";
        break;
    case Operator::JMPLT:
        comparison = ">=";
        break;
    case Operator::JMPLE:
        comparison = ">";
        break;
    case Operator::JMPGT:
        comparison = "<=";
        break;
    case Operator::JMPGE:
        comparison = "<";
        break;
    case Operator::JMPT:
        comparison = "~=";
        break;
    case Operator::JMPF:
    case Operator::JMPONF:
        comparison = "==";
        break;
    default:
        printf("OP %d not covered for conditions\n", (int)op);
        exit(1);
    }

    Vector<Expression> operands;

    if(op >= Operator::JMPNE && op <= Operator::JMPGE)
    {
        auto left = std::get<Expression>(state.stack.back());
        state.stack.pop_back();

        auto right = std::get<Expression>(state.stack.back());
        state.stack.pop_back();

        operands = {left, right};
    }
    else if(op >= Operator::JMPT && op <= Operator::JMPONF)
    {
        auto left = std::get<Expression>(state.stack.back());
        state.stack.pop_back();

        operands = {left, Identifier("nil")};
    }

    // if block
    if(!ast->context.is_condition || ast->context.jmp_offset == 0)
    {
        const auto operation = AstOperation(comparison, operands);
        const auto block     = ConditionBlock(operation, Vector<Statement>{});
        const auto condition = Condition(Vector<ConditionBlock>{block});
        ast->statements.push_back(condition);

        enter_block(ast);
        ast->context.is_condition = true;
        ast->context.jump_offset  = state.PC + S(instruction);
    }
    // elseif block
    else
    {
        auto&      condition = std::get<Condition>(ast->parent->statements.back());
        const auto operation = AstOperation(comparison, operands);
        const auto block     = ConditionBlock(operation, Vector<Statement>{});
        condition.blocks.push_back(block);

        ast->context.jump_offset = state.PC + S(instruction);
    }
    ast->context.is_jmp_block = false;

    return Error::NONE;
}

/*
 * Arguments:       J
 * Stack before:    x
 * Stack after:     (x ~= nil) ? x : -
 * Side effects:    (x ~= nil) ? PC += s
 *
 * @brief
 */
Error handle_jmpont(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    AstOperation operation("or", {left});
    state.stack.push_back(operation);

    ast->context.is_or_block = true;
    ast->context.jump_offset = state.PC + S(instruction);

    return Error::NONE;
}

/*
 * Arguments:       J
 * Stack before:    -
 * Stack after:     -
 * Side effects:    PC += s
 *
 * @brief
 */
Error handle_jmp(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    if(ast->context.is_condition)
    {
        auto& condition = std::get<Condition>(ast->parent->statements.back());
        condition.blocks.back().statements = ast->statements;
        ast->statements.clear();

        ast->context.jump_offset  = state.PC + S(instruction);
        ast->context.jmp_offset   = state.PC + S(instruction);
        ast->context.is_jmp_block = true;
    }

    return Error::NONE;
}

/*
 * Arguments:       -
 * Stack before:    -
 * Stack after:     nil
 * Side effects:    PC++
 *
 * @brief
 */
Error handle_push_niljump(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    return Error::NONE;
}

/*
 * Arguments:       J
 * Stack before:    -
 * Stack after:     -
 * Side effects:    -
 *
 * @brief
 */
Error handle_forprep(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto counter = get_local_from_pc(function, state.PC, 0);

    const auto increment = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    const auto end = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    const auto begin = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    ForLoop loop(counter, begin, end, increment, Vector<Statement>{});
    ast->statements.push_back(loop);

    enter_block(ast);

    return Error::NONE;
}

/*
 * Arguments:       J
 * Stack before:    -
 * Stack after:     -
 * Side effects:    -
 *
 * @brief
 */
Error handle_lforprep(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto key   = get_local_from_pc(function, state.PC, 1);
    const auto value = get_local_from_pc(function, state.PC, 2);

    const auto right = std::get<Identifier>(std::get<Expression>(state.stack.back())).name;
    state.stack.pop_back();

    ForInLoop loop(key, value, right, Vector<Statement>{});
    ast->statements.push_back(loop);

    enter_block(ast);

    return Error::NONE;
}

/*
 * Arguments:       J
 * Stack before:    -
 * Stack after:     -
 * Side effects:    -
 *
 * @brief
 */
Error handle_forloop(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    auto& loop      = std::get<ForLoop>(ast->parent->statements.back());
    loop.statements = ast->statements;

    exit_block(ast);

    return Error::NONE;
}

/*
 * Arguments:       J
 * Stack before:    -
 * Stack after:     -
 * Side effects:    -
 *
 * @brief
 */
Error handle_lforloop(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    auto& loop      = std::get<ForInLoop>(ast->parent->statements.back());
    loop.statements = ast->statements;

    exit_block(ast);

    return Error::NONE;
}

/*
 * Arguments:       A B
 * Stack before:    v_b - v_1
 * Stack after:     closure(KPROTO[a], v_1, ...,  v_b)
 * Side effects:    -
 *
 * @brief   A new inline closure is defined. The closure is at position 'A' in the global
 *          chunk. The arguments of the closure are defined implicitly by the number of
 *          their PC start.
 */
Error handle_closure(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto a = A(instruction);

    enter_block(ast);

    // Each closure needs a new state.
    auto new_state = State();
    parse_function(new_state, ast, function.functions[a]);

    // Arguments of the closure have to be searched in the local table.
    Vector<Identifier> arguments;
    for(const auto& local : function.functions[a].locals)
    {
        // Locals that start from PC = 0 are closure arguments.
        if(local.start_pc == 0)
        {
            arguments.push_back(Identifier(local.name));
        }
    }

    exit_block(ast);

    state.stack.push_back(Closure(ast->child->statements, arguments));

    return Error::NONE;
}

// Public functions

Error parse_function(State& state, Ast*& ast, const Function& function)
{
    // Arguments of the function if it is a closure, otherwise just locals
    for(const auto& local : function.locals)
    {
        if(local.start_pc == 0)
        {
            const auto name = Identifier(local.name);
            state.stack.push_back(name);
        }
    }

    for(const auto& i : function.instructions)
    {
        auto op = Operator(OP(i));

        if(TABLE.count(op) == 0)
        {
            printf("DEBUG: No action for %d (%s) in table\n", (int)op, OP_TO_STR[op].c_str());
            return Error::NONE;  // TODO
        }

        // Local lifetime is defined by the PC range. If the PC hits the start PC of a
        // local variable a local assignment is appended to the program. More than one
        // variable might be defined in one line.
        // The local variable definition is not encoded in the bytecode and has to be
        // handled separately from the ActionTable.
        if(state.PC > 0 && state.stack.size())
        {
            // Identify all the locals that are defined in this line.
            Vector<Identifier> locals;
            for(const auto& local : function.locals)
            {
                if(local.start_pc == state.PC)
                {
                    locals.push_back(Identifier(local.name));
                }
            }

            // Make a local assignment is a local is defined in this line.
            if(locals.size())
            {
                const auto value = std::get<Expression>(state.stack.back());
                state.stack.pop_back();
                ast->statements.push_back(LocalAssignment(locals, value));

                // Locals need to be pushed onto the stack because of the offset
                // (S register) of the CALL operator.
                for(const auto& local : locals)
                    state.stack.push_back(local);
            }
        }

        // Run the parsing function for the current operator.
        const auto result = TABLE[op](state, ast, i, function);

        // Return on error.
        if(result != Error::NONE)
            return result;

        // Conditional statements are handled implicitly through the jump offset
        // (S register) of the instruction. The offset has to be checked if the parser
        // is currently inside a condition block.
        if(state.PC == ast->context.jump_offset)
        {
            // Inline or comparison for an assignment (x = x or y)
            if(ast->context.is_or_block)
            {
                auto right = std::get<Expression>(state.stack.back());
                state.stack.pop_back();

                auto& operation =
                    std::get<AstOperation>(std::get<Expression>(state.stack.back()));
                operation.ex.insert(operation.ex.begin(), right);

                ast->context.is_or_block = false;
            }
            // Handle the end of a condition block if the PC is right.
            else if(ast->context.is_condition)
            {
                while(state.PC == ast->context.jump_offset)
                {
                    auto& condition = std::get<Condition>(ast->parent->statements.back());

                    // Create an else block if the last jump operator was a JMP
                    if(ast->context.is_jmp_block)
                    {
                        const auto operation = AstOperation("", Vector<Expression>{});
                        const auto block = ConditionBlock(operation, Vector<Statement>{});
                        condition.blocks.push_back(block);
                    }

                    condition.blocks.back().statements = ast->statements;
                    ast->statements.clear();

                    ast->context.is_condition = false;
                    exit_block(ast);
                }
            }
        }
        state.PC++;
    }

    return Error::NONE;
}
