#include "parser/parser.hpp"

#include <algorithm>
#include <optional>

void State::print()
{
    size_t i = 0;

    static const std::unordered_map<size_t, std::string> STATEMENT_VARIANTS = {
        {i++, "Assignment"},
        {i++, "Call"},
        {i++, "Condition"},
        {i++, "ForLoop"},
        {i++, "ForInLoop"},
        {i++, "ForInLoop"},
        {i++, "LocalDefinition"},
        {i++, "Return"},
        {i++, "TailCall"},
        {i++, "WhileLoop"},
    };

    i = 0;

    static const std::unordered_map<size_t, std::string> EXPRESSION_VARIANTS = {
        {i++, "Call"},
        {i++, "Closure"},
        {i++, "Dotted"},
        {i++, "Identifier"},
        {i++, "Indexed"},
        {i++, "AstInt"},
        {i++, "AstList"},
        {i++, "AstMap"},
        {i++, "AstNumber"},
        {i++, "AstOperation"},
        {i++, "AstString"},
        {i++, "AstTable"},
    };

    printf("Stack:\n");
    for(const auto& el : stack)
    {
        printf("  ");
        switch(el.index())
        {
        case 0:
        {
            auto statement = std::get<Statement>(el);
            printf("%s\n", STATEMENT_VARIANTS.at(el.index()).c_str());
            break;
        }
        case 1:
        {
            auto expression = std::get<Expression>(el);
            printf("%s\n", EXPRESSION_VARIANTS.at(el.index()).c_str());
            break;
        }
        }
    }
}

Status handle_condition(State&, Ast*&, const Instruction&, const Function&);

Status handle_end(State&, Ast*&, const Instruction&, const Function&);
Status handle_return(State&, Ast*&, const Instruction&, const Function&);
Status handle_call(State&, Ast*&, const Instruction&, const Function&);
Status handle_tail_call(State&, Ast*&, const Instruction&, const Function&);
Status handle_push_nil(State&, Ast*&, const Instruction&, const Function&);
Status handle_pop(State&, Ast*&, const Instruction&, const Function&);
Status handle_push_int(State&, Ast*&, const Instruction&, const Function&);
Status handle_push_string(State&, Ast*&, const Instruction&, const Function&);
Status handle_push_num(State&, Ast*&, const Instruction&, const Function&);
Status handle_push_neg_num(State&, Ast*&, const Instruction&, const Function&);
Status handle_push_upvalue(State&, Ast*&, const Instruction&, const Function&);
Status handle_get_local(State&, Ast*&, const Instruction&, const Function&);
Status handle_get_global(State&, Ast*&, const Instruction&, const Function&);
Status handle_get_table(State&, Ast*&, const Instruction&, const Function&);
Status handle_get_dotted(State&, Ast*&, const Instruction&, const Function&);
Status handle_get_indexed(State&, Ast*&, const Instruction&, const Function&);
Status handle_push_self(State&, Ast*&, const Instruction&, const Function&);
Status handle_create_table(State&, Ast*&, const Instruction&, const Function&);
Status handle_set_local(State&, Ast*&, const Instruction&, const Function&);
Status handle_set_global(State&, Ast*&, const Instruction&, const Function&);
Status handle_set_table(State&, Ast*&, const Instruction&, const Function&);
Status handle_set_list(State&, Ast*&, const Instruction&, const Function&);
Status handle_set_map(State&, Ast*&, const Instruction&, const Function&);
Status handle_add(State&, Ast*&, const Instruction&, const Function&);
Status handle_addi(State&, Ast*&, const Instruction&, const Function&);
Status handle_sub(State&, Ast*&, const Instruction&, const Function&);
Status handle_mult(State&, Ast*&, const Instruction&, const Function&);
Status handle_div(State&, Ast*&, const Instruction&, const Function&);
Status handle_pow(State&, Ast*&, const Instruction&, const Function&);
Status handle_concat(State&, Ast*&, const Instruction&, const Function&);
Status handle_minus(State&, Ast*&, const Instruction&, const Function&);
Status handle_not(State&, Ast*&, const Instruction&, const Function&);
Status handle_jmpne(State&, Ast*&, const Instruction&, const Function&);
Status handle_jmpeq(State&, Ast*&, const Instruction&, const Function&);
Status handle_jmplt(State&, Ast*&, const Instruction&, const Function&);
Status handle_jmple(State&, Ast*&, const Instruction&, const Function&);
Status handle_jmpgt(State&, Ast*&, const Instruction&, const Function&);
Status handle_jmpge(State&, Ast*&, const Instruction&, const Function&);
Status handle_jmpt(State&, Ast*&, const Instruction&, const Function&);
Status handle_jmpf(State&, Ast*&, const Instruction&, const Function&);
Status handle_jmpont(State&, Ast*&, const Instruction&, const Function&);
Status handle_jmponf(State&, Ast*&, const Instruction&, const Function&);
Status handle_jmp(State&, Ast*&, const Instruction&, const Function&);
Status handle_push_niljump(State&, Ast*&, const Instruction&, const Function&);
Status handle_forprep(State&, Ast*&, const Instruction&, const Function&);
Status handle_lforprep(State&, Ast*&, const Instruction&, const Function&);
Status handle_forloop(State&, Ast*&, const Instruction&, const Function&);
Status handle_lforloop(State&, Ast*&, const Instruction&, const Function&);
Status handle_closure(State&, Ast*&, const Instruction&, const Function&);

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
    {Operator::JMPNE,       &handle_jmpne},
    {Operator::JMPEQ,       &handle_jmpeq},
    {Operator::JMPLT,       &handle_jmplt},
    {Operator::JMPLE,       &handle_jmple},
    {Operator::JMPGT,       &handle_jmpgt},
    {Operator::JMPGE,       &handle_jmpge},
    {Operator::JMPT,        &handle_jmpt},
    {Operator::JMPF,        &handle_jmpf},
    {Operator::JMPONT,      &handle_jmpont},
    {Operator::JMPONF,      &handle_jmponf},
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

Status enter_block(State& state, Ast*& ast)
{
    auto* child   = new Ast();
    child->parent = ast;

    ast->child = child;
    ast        = child;

    state.scope_level += 1;

    return Status::OK;
}

Status exit_block(State& state, Ast*& ast)
{
    if(ast->parent)
        ast = ast->parent;

    state.scope_level -= 1;

    return Status::OK;
}

/*
 * @brief   Each conditional operator can have 0, 1, or 2 arguments.
 *          The operation determines how the arguments are handled.
 */
Status handle_condition(
    State&                    state,
    Ast*&                     ast,
    const Instruction&        instruction,
    const String&             comparison,
    const Vector<Expression>& operands)
{
    // if block
    if(!ast->context.is_condition || ast->context.jmp_offset == 0)
    {
        const auto operation = AstOperation(comparison, operands);
        const auto block     = ConditionBlock(operation, {});
        const auto condition = Condition({block});
        ast->statements.push_back(condition);

        enter_block(state, ast);
        ast->context.is_condition = true;
        ast->context.jump_offset  = state.PC + S(instruction);
    }
    // elseif block
    else
    {
        auto&      condition = std::get<Condition>(ast->parent->statements.back());
        const auto operation = AstOperation(comparison, operands);
        const auto block     = ConditionBlock(operation, {});
        condition.blocks.push_back(block);

        ast->context.jump_offset = state.PC + S(instruction);
    }
    ast->context.is_jmp_block = false;

    return Status::OK;
}

/*
 * @brief   Assignment may spread accross multiple instructions. The Assignment statement
 *          has therefore be stored and reused conditionally. This is the case if multiple
 *          values are pushed to the stack before the first SET_LOCAL or SET_GLOBAL
 * instruction appears. Expressions like a function may return multiples values so it has
 * to be kept track of how many values are actually on the right side. The values have to
 * be 'adjusted' when reassembling the statement.
 */
Status handle_assignment(State& state, Ast*& ast, const Identifier& left)
{
    auto values_on_stack = state.stack.size() - state.reserved_elements;
    if(values_on_stack > 0)
    {
        // Get all values that were previously pushed onto the stack.
        Vector<Expression> values;
        while(values_on_stack > 0)
        {
            values.push_back(std::get<Expression>(state.stack.back()));
            state.stack.pop_back();
            --values_on_stack;
        }

        Assignment ass({left}, values, 1, values.size());
        ast->statements.push_back(ass);
    }
    else
    {
        // A function call with multiple return values represents the right.
        if(std::holds_alternative<Assignment>(ast->statements.back()))
        {
            auto& ass = std::get<Assignment>(ast->statements.back());
            ass.left.push_back(left);
        }
        else
        {
            return Status::BAD_VARIANT;
        }
    }

    return Status::OK;
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
Status handle_end(State&, Ast*&, const Instruction&, const Function&)
{
    return Status::OK;
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
Status handle_return(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
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

    return Status::OK;
}

/*
 * Arguments:       A B
 * Stack before:    v_n - v_1  f (at A)     | function name is a position 'A'
 * Stack after:     r_b - r_1
 * Side effects:    f(v1, ..., v_n)
 *
 * @brief   Pops elements from the stack until it has a size of 'A'. The last popped
 *          element is the caller. The rest of the popped elements are the arguments
 *          in reversed order.
 *          In case the caller is a table or a map (both of type AstTable) we just push
 *          it back onto the stack.
 */
Status handle_call(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto a = A(instruction);  // The caller is at position a
    const auto b = B(instruction);  // > 0 if it is an expression call returning b
                                    // arguments.

    Vector<Expression> args;
    while(state.stack.size() > a + 1)
    {
        args.push_back(std::get<Expression>(state.stack.back()));
        state.stack.pop_back();
    }

    auto caller = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    if(std::holds_alternative<AstTable>(caller))
    {
        auto table = std::get<AstTable>(caller);
        state.stack.push_back(table);
    }
    else
    {
        std::reverse(args.begin(), args.end());

        if(b == 0)
            ast->statements.push_back(Call({caller}, args));
        else
            state.stack.push_back(Expression(Call({caller}, args, b)));
    }

    return Status::OK;
}

/*
 * Arguments:       A B
 * Stack before:    v_n - v_1  f (at A)     | function name is a position 'A'
 * Stack after:     return
 * Side effects:    f(v1, ..., v_n)
 *
 * @brief   Special case of the call operator. However it is only used for proper function
 *          calls. The operator pops elements from the stack until it has a size of 'A'.
 *          The last popped element is the function caller. The rest of the popped
 *          elements are the arguments in reversed order. The value of the function is
 *          returned from the current closure.
 */
Status handle_tail_call(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto a = A(instruction);  // The caller is at position a

    Vector<Expression> args;
    while(state.stack.size() > a + 1)
    {
        args.push_back(std::get<Expression>(state.stack.back()));
        state.stack.pop_back();
    }

    auto caller = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    std::reverse(args.begin(), args.end());

    ast->statements.push_back(TailCall({caller}, args));

    return Status::OK;
}

/*
 * Arguments:       U
 * Stack before:    -
 * Stack after:     nil_1 - nil_u
 * Side effects:    -
 *
 * @brief   Pushes one or multiple nil values on to the stack.
 */
Status handle_push_nil(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    auto u = U(instruction);

    for(auto i = u; i > 0; --i)
    {
        state.stack.push_back(Identifier("nil"));
    }

    return Status::OK;
}

/*
 * Arguments:       U
 * Stack before:    a_u - a_1
 * Stack after:     -
 * Side effects:    -
 *
 * @brief   Pops one or multiple values from the stack.
 */
Status handle_pop(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    auto u = U(instruction);

    for(auto i = u; i > 0; --i)
    {
        state.stack.pop_back();
    }

    return Status::OK;
}

/*
 * Arguments:       S
 * Stack before:    -
 * Stack after:     (Number)s
 * Side effects:    -
 *
 * @brief
 */
Status handle_push_int(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    const auto s = S(instruction);

    state.stack.push_back(AstInt(s));

    return Status::OK;
}

/*
 * Arguments:       K       | K/U used as index for the globals table
 * Stack before:    -
 * Stack after:     KSTR[k]
 * Side effects:    -
 *
 * @brief
 */
Status handle_push_string(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto k      = U(instruction);
    const auto string = function.globals[k];

    state.stack.push_back(AstString(string));

    return Status::OK;
}

/*
 * Arguments:       N       | N/U used as index for the number table
 * Stack before:    -
 * Stack after:     KNUM[n]
 * Side effects:    -
 *
 * @brief
 */
Status handle_push_num(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto n      = U(instruction);
    const auto number = function.numbers[n];

    state.stack.push_back(AstNumber(number));

    return Status::OK;
}

/*
 * Arguments:       N       | N/U used as index for the number table
 * Stack before:    -
 * Stack after:     -KNUM[n]
 * Side effects:    -
 *
 * @brief
 */
Status handle_push_neg_num(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto n      = U(instruction);
    const auto number = function.numbers[n];

    state.stack.push_back(AstNumber(-number));

    return Status::OK;
}

/*
 * Arguments:       U
 * Stack before:    -
 * Stack after:     Closure[u]
 * Side effects:    -
 *
 * @brief
 */
Status handle_push_upvalue(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    // TODO
    return Status::OK;
}

/*
 * Arguments:       L       | L/U used as index for the local table
 * Stack before:    -
 * Stack after:     LOC[l]
 * Side effects:    -
 *
 * @brief   Pushes the l-th valid local onto the stack. The index of the local has to
 *          be normalized according to the validity range.
 */
Status handle_get_local(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    auto l = U(instruction);

    auto index = 0;
    auto i     = 0;
    while(i != l)
    {
        if(function.locals[index].start_pc <= state.PC &&
           function.locals[index].end_pc >= state.PC)
            i++;
        index++;
    }

    auto name = function.locals[index].name;

    state.stack.push_back(Identifier(name));

    return Status::OK;
}

/*
 * Arguments:       K       | K/U used as index for the globals table
 * Stack before:    -
 * Stack after:     VAR[KSTR[k]]
 * Side effects:    -
 *
 * @brief
 */
Status handle_get_global(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto k    = U(instruction);
    const auto name = function.globals[k];

    state.stack.push_back(Identifier(name));

    return Status::OK;
}

/*
 * Arguments:       -
 * Stack before:    i t
 * Stack after:     t[i]
 * Side effects:    -
 *
 * @brief
 */
Status handle_get_table(State& state, Ast*& ast, const Instruction&, const Function&)
{
    // i
    const auto index = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    // t
    const auto table = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    state.stack.push_back(Indexed({table, index}));

    return Status::OK;
}

/*
 * Arguments:       K
 * Stack before:    t
 * Stack after:     t[KSTR[k]]
 * Side effects:    -
 *
 * @brief
 */
Status handle_get_dotted(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto k    = U(instruction);
    const auto name = function.globals[k];

    // t
    const auto table = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    state.stack.push_back(Dotted({table, Identifier(name)}));

    return Status::OK;
}

/*
 * Arguments:       L
 * Stack before:    t
 * Stack after:     t[LOC[l]]
 * Side effects:    -
 *
 * @brief
 */
Status handle_get_indexed(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto l    = U(instruction);
    const auto name = function.locals[l].name;

    // t
    const auto table = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    state.stack.push_back(Indexed({table, Identifier(name)}));

    return Status::OK;
}

/*
 * Arguments:       K
 * Stack before:    t
 * Stack after:     t t[KSTR[k]]
 * Side effects:    -
 *
 * @brief
 */
Status handle_push_self(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto k    = U(instruction);
    const auto name = function.globals[k];

    // t
    const auto table = std::get<Expression>(state.stack.back());
    state.stack.push_back(Dotted({table, Identifier(name)}));

    return Status::OK;
}

/*
 * Arguments:       U
 * Stack before:    -
 * Stack after:     newarray(size = u)
 * Side effects:    -
 *
 * @brief   Creates a new table element (may be list or map) of the given size.
 */
Status handle_create_table(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    // Its only a table if an identifier is on the stack before. Otherwise its a map or
    // list.

    std::string name;
    if(state.stack.size() > state.reserved_elements)
    {
        const auto ex = std::get<Expression>(state.stack.back());
        if(std::holds_alternative<Identifier>(ex))
        {
            name = std::get<Identifier>(ex).name;
            state.stack.pop_back();
        }
    }

    const auto u = U(instruction);
    AstTable   table(u, name, {});
    state.stack.push_back(table);

    return Status::OK;
}

/*
 * Arguments:       L
 * Stack before:    x
 * Stack after:     -
 * Side effects:    LOC[l] = x
 *
 * @brief   Sets the local variable at position l to the top-most value on the stack.
 */
Status handle_set_local(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto l    = U(instruction);
    const auto left = Identifier(function.locals[l].name);

    return handle_assignment(state, ast, left);
}

/*
 * Arguments:       K
 * Stack before:    x
 * Stack after:     -
 * Side effects:    VAR[KSTR[k]] = x
 *
 * @brief   Creates an assignment statement.
 */
Status handle_set_global(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto k    = U(instruction);
    const auto left = Identifier(function.globals[k]);

    return handle_assignment(state, ast, left);
}

/*
 * Arguments:       A B
 * Stack before:    v a_a - a_1 i t
 * Stack after:     (pops b values)
 * Side effects:    t[i] = v
 *
 * @brief   Creates a table assignment with b table elements.
 */
Status handle_set_table(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto b = B(instruction);

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

    Assignment ass({Identifier(left)}, {right});
    ast->statements.push_back(ass);

    return Status::OK;
}

/*
 * Arguments:       A B
 * Stack before:    v_b-v_1 t
 * Stack after:     t
 * Side effects:    t[i+a*FPF] = v_i
 *
 * @brief   Creates a list of b elements that is pushed onto the stack.
 */
Status handle_set_list(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    const auto b = B(instruction);

    Vector<Expression> list;
    for(unsigned i = 0; i < b; ++i)
    {
        list.push_back(std::get<Expression>(state.stack.back()));
        state.stack.pop_back();
    }

    std::reverse(list.begin(), list.end());

    state.stack.pop_back();  // empty AstTable
    state.stack.push_back(AstList(list));

    return Status::OK;
}

/*
 * Arguments:       U
 * Stack before:    v_u k_u - v_1 k_1 t
 * Stack after:     t
 * Side effects:    t[k_i] = v_i
 *
 * @brief   Sets the previous u pushed elements as values for a table if a table
 *          was created before. Otherwise a map of this size is created and pushed
 *          onto the stack.
 */
Status handle_set_map(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    const auto u = U(instruction);

    Vector<std::pair<Expression, Expression>> map;
    for(unsigned i = 0; i < u; ++i)
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

    return Status::OK;
}

/*
 * Arguments:       -
 * Stack before:    y x
 * Stack after:     x + y
 * Side effects:    -
 *
 * @brief
 */
Status handle_add(State& state, Ast*& ast, const Instruction&, const Function&)
{
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    state.stack.push_back(AstOperation("+", {left, right}));

    return Status::OK;
}

/*
 * Arguments:       S
 * Stack before:    x
 * Stack after:     x + s
 * Side effects:    -
 *
 * @brief
 */
Status handle_addi(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    const auto s     = S(instruction);
    const auto right = AstNumber(s);

    state.stack.push_back(AstOperation("+", {left, right}));

    return Status::OK;
}

/*
 * Arguments:       -
 * Stack before:    y x
 * Stack after:     x - y
 * Side effects:    -
 *
 * @brief
 */
Status handle_sub(State& state, Ast*& ast, const Instruction&, const Function&)
{
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    state.stack.push_back(AstOperation("-", {left, right}));

    return Status::OK;
}

/*
 * Arguments:       -
 * Stack before:    y x
 * Stack after:     x * y
 * Side effects:    -
 *
 * @brief
 */
Status handle_mult(State& state, Ast*& ast, const Instruction&, const Function&)
{
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    state.stack.push_back(AstOperation("*", {left, right}));

    return Status::OK;
}

/*
 * Arguments:       -
 * Stack before:    y x
 * Stack after:     x / y
 * Side effects:    -
 *
 * @brief
 */
Status handle_div(State& state, Ast*& ast, const Instruction&, const Function&)
{
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    state.stack.push_back(AstOperation("/", {left, right}));

    return Status::OK;
}

/*
 * Arguments:       -
 * Stack before:    y x
 * Stack after:     x ^ y
 * Side effects:    -
 *
 * @brief
 */
Status handle_pow(State& state, Ast*& ast, const Instruction&, const Function&)
{
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    state.stack.push_back(AstOperation("^", {left, right}));

    return Status::OK;
}

/*
 * Arguments:       U
 * Stack before:    v_u - v_1
 * Stack after:     v1 - v_u
 * Side effects:    -
 *
 * @brief   Concatenates u elements from the stack together.
 */
Status handle_concat(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    const auto         u = U(instruction);
    Vector<Expression> expressions;
    for(unsigned i = 0; i < u; ++i)
    {
        expressions.push_back(std::get<Expression>(state.stack.back()));
        state.stack.pop_back();
    }

    std::reverse(expressions.begin(), expressions.end());
    state.stack.push_back(AstOperation("..", expressions));

    return Status::OK;
}

/*
 * Arguments:       -
 * Stack before:    x
 * Stack after:     -x
 * Side effects:    -
 *
 * @brief   Negates the numeric value of the top-most element on the stack.
 */
Status handle_minus(State& state, Ast*& ast, const Instruction&, const Function&)
{
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    state.stack.push_back(AstOperation("-", {right}));

    return Status::OK;
}

/*
 * Arguments:       -
 * Stack before:    x
 * Stack after:     (x == nil) ? 1 : nil
 * Side effects:    -
 *
 * @brief   Negates the truth value of the top-most element on the stack.
 */
Status handle_not(State& state, Ast*& ast, const Instruction&, const Function&)
{
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    state.stack.push_back(AstOperation("not ", {right}));

    return Status::OK;
}

/*
 * Arguments:       J
 * Stack before:    y x
 * Stack after:     -
 * Side effects:    (x ~= y) ? PC += s
 *
 * @brief
 */
Status handle_jmpne(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    return handle_condition(state, ast, instruction, "==", {left, right});
}

/*
 * Arguments:       J
 * Stack before:    y x
 * Stack after:     -
 * Side effects:    (x == y) ? PC += s
 *
 * @brief
 */
Status handle_jmpeq(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    return handle_condition(state, ast, instruction, "~=", {left, right});
}

/*
 * Arguments:       J
 * Stack before:    y x
 * Stack after:     -
 * Side effects:    (x < y) ? PC += s
 *
 * @brief
 */
Status handle_jmplt(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    return handle_condition(state, ast, instruction, ">=", {left, right});
}

/*
 * Arguments:       J
 * Stack before:    y x
 * Stack after:     -
 * Side effects:    (x <= y) ? PC += s
 *
 * @brief
 */
Status handle_jmple(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    return handle_condition(state, ast, instruction, ">", {left, right});
}

/*
 * Arguments:       J
 * Stack before:    y x
 * Stack after:     -
 * Side effects:    (x > y) ? PC += s
 *
 * @brief
 */
Status handle_jmpgt(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    return handle_condition(state, ast, instruction, "<=", {left, right});
}

/*
 * Arguments:       J
 * Stack before:    y x
 * Stack after:     -
 * Side effects:    (x >= y) ? PC += s
 *
 * @brief
 */
Status handle_jmpge(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    return handle_condition(state, ast, instruction, "<", {left, right});
}

/*
 * Arguments:       J
 * Stack before:    x
 * Stack after:     -
 * Side effects:    (x ~= nil) ? PC += s
 *
 * @brief
 */
Status handle_jmpt(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    return handle_condition(state, ast, instruction, "~=", {left, Identifier("nil")});
}

/*
 * Arguments:       J
 * Stack before:    x
 * Stack after:     -
 * Side effects:    (x == nil) ? PC += s
 *
 * @brief
 */
Status handle_jmpf(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    return handle_condition(state, ast, instruction, "==", {left, Identifier("nil")});
}

/*
 * Arguments:       J
 * Stack before:    x
 * Stack after:     (x ~= nil) ? x : -
 * Side effects:    (x ~= nil) ? PC += s
 *
 * @brief
 */
Status handle_jmpont(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    AstOperation operation("or", {right});
    state.stack.push_back(operation);

    ast->context.is_or_block = true;
    ast->context.jump_offset = state.PC + S(instruction);

    return Status::OK;
}

/*
 * Arguments:       J
 * Stack before:    x
 * Stack after:     (x == nil) ? x : -
 * Side effects:    (x == nil) ? PC += s
 *
 * @brief
 */
Status handle_jmponf(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    return handle_condition(state, ast, instruction, "==", {left, Identifier("nil")});
}

/*
 * Arguments:       J
 * Stack before:    -
 * Stack after:     -
 * Side effects:    PC += s
 *
 * @brief
 */
Status handle_jmp(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    if(ast->context.is_condition && state.PC >= ast->context.jump_offset)
    {
        auto& condition = std::get<Condition>(ast->parent->statements.back());

        // Create an else block if the last jump operator was a JMP
        if(ast->context.is_jmp_block)
        {
            const auto operation = AstOperation("", {});
            const auto block     = ConditionBlock(operation, {});
            condition.blocks.push_back(block);
        }

        condition.blocks.back().statements = ast->statements;
        ast->statements.clear();

        ast->context.is_condition = false;
        exit_block(state, ast);
    }

    if(ast->context.is_condition)
    {
        auto& condition = std::get<Condition>(ast->parent->statements.back());
        condition.blocks.back().statements = ast->statements;
        ast->statements.clear();

        ast->context.jump_offset  = state.PC + S(instruction);
        ast->context.jmp_offset   = state.PC + S(instruction);
        ast->context.is_jmp_block = true;
    }

    return Status::OK;
}

/*
 * Arguments:       -
 * Stack before:    -
 * Stack after:     nil
 * Side effects:    PC++
 *
 * @brief
 */
Status handle_push_niljump(State& state, Ast*& ast, const Instruction& instruction, const Function&)
{
    state.stack.push_back(Identifier("nil"));
    return Status::OK;
}

/*
 * Arguments:       J
 * Stack before:    -
 * Stack after:     -
 * Side effects:    -
 *
 * @brief Counter, begin, limit, and step are declared in the next instruction.
 *        Therefore, we declare placeholder values and assign the real values
 *        when we reach the end of the loop.
 */
Status handle_forprep(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    ForLoop loop("", Identifier(""), Identifier(""), Identifier(""), {});
    ast->statements.push_back(loop);

    enter_block(state, ast);

    return Status::OK;
}

/*
 * Arguments:       J
 * Stack before:    -
 * Stack after:     -
 * Side effects:    -
 *
 * @brief Key, value, and table are declared in the next instruction.
 *        Therefore, we declare placeholder values and assign the real values
 *        when we reach the end of the loop.
 */
Status handle_lforprep(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    state.stack.push_back(Identifier(""));  // value
    state.stack.push_back(Identifier(""));  // key

    ForInLoop loop("", "", Identifier(""), {});
    ast->statements.push_back(loop);

    enter_block(state, ast);

    return Status::OK;
}

/*
 * Arguments:       J
 * Stack before:    -
 * Stack after:     -
 * Side effects:    -
 *
 * @brief Once we have reached the end of the loop we extract the loop variable
 *        definitions (key, value, table) from the first statement (the local
 *        definition) and remove it from the statements.
 */
Status handle_forloop(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto nested_statements = ast->statements;
    const auto loop_variables    = std::get<LocalDefinition>(nested_statements.front());

    exit_block(state, ast);

    auto& loop     = std::get<ForLoop>(ast->statements.back());
    loop.counter   = loop_variables.left[0].name;
    loop.begin     = loop_variables.right[0];
    loop.end       = loop_variables.right[1];
    loop.increment = loop_variables.right[2];
    loop.statements =
        Vector<Statement>(nested_statements.begin() + 1, nested_statements.end());

    state.stack.pop_back();
    state.stack.pop_back();
    state.stack.pop_back();

    return Status::OK;
}

/*
 * Arguments:       J
 * Stack before:    -
 * Stack after:     -
 * Side effects:    -
 *
 * @brief Once we have reached the end of the loop we extract the loop variable
 *        definitions (key, value, table) from the first statement (the local
 *        definition) and remove it from the statements.
 */
Status handle_lforloop(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto nested_statements = ast->statements;
    const auto loop_variables    = std::get<LocalDefinition>(nested_statements.front());

    exit_block(state, ast);

    auto& loop = std::get<ForInLoop>(ast->statements.back());
    loop.table = loop_variables.right[0];
    loop.key   = loop_variables.left[1].name;
    loop.value = loop_variables.left[2].name;
    loop.statements =
        Vector<Statement>(nested_statements.begin() + 1, nested_statements.end());

    state.stack.pop_back();
    state.stack.pop_back();
    state.stack.pop_back();

    return Status::OK;
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
Status handle_closure(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto a = A(instruction);

    enter_block(state, ast);

    // Each closure needs a new state.
    auto new_state = State();
    auto error     = parse_function(new_state, ast, function.functions[a]);

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

    exit_block(state, ast);

    state.stack.push_back(Closure(ast->child->statements, arguments));

    return error;
}

// Public functions

Status parse_function(State& state, Ast*& ast, const Function& function)
{
    // Lookup table for locals based on their starting lifetime.
    std::unordered_map<unsigned, Vector<unsigned>> local_spawn;
    std::unordered_map<unsigned, Vector<unsigned>> local_kill;

    unsigned local_index = 0;
    for(const auto& local : function.locals)
    {
        if(local.start_pc == 0)
        {
            state.stack.push_back(Identifier(local.name));
            state.reserved_elements += 1;
        }

        if(local_spawn.count(local.start_pc) == 0)
            local_spawn[local.start_pc] = {};

        if(local_kill.count(local.end_pc) == 0)
            local_kill[local.end_pc] = {};

        local_spawn[local.start_pc].push_back(local_index);
        local_kill[local.end_pc].push_back(local_index);

        local_index++;
    }

    for(const auto& i : function.instructions)
    {
        auto op = Operator(OP(i));

        unsigned locals_defined = 0;

        // Local lifetime is defined by the PC range. If the PC hits the start PC of a
        // local variable a local definition is appended to the program. More than one
        // variable might be defined in one line.
        // However, on scope exit the VM pops the 'killed' local from the stack.
        // The local variable definition is not encoded in the bytecode and has to be
        // handled separately from the ActionTable.
        if(state.PC > 0)
        {
            if(local_spawn[state.PC].size() > 0)
            {
                for(const auto& index : local_spawn[state.PC])
                {
                    locals_defined += 1;
                }
            }
            if(local_kill[state.PC].size() > 0)
            {
                for(const auto& index : local_kill[state.PC])
                {
                    state.reserved_elements -= 1;
                }
            }

            if(locals_defined > 0)
            {
                // Pop the value(s) assigned to that local from the stack.
                auto values = Vector<Expression>();
                for(auto l = state.stack.size() - state.reserved_elements; l > 0; --l)
                {
                    values.push_back(std::get<Expression>(state.stack.back()));
                    state.stack.pop_back();
                }

                // Collect the local names and push them onto the stack.
                auto locals = Vector<Identifier>();
                for(const auto& index : local_spawn[state.PC])
                {
                    auto local = Identifier(function.locals[index].name);
                    locals.push_back(local);
                    state.stack.push_back(local);
                    state.reserved_elements += 1;
                }

                // Make the local definition
                std::reverse(values.begin(), values.end());
                auto definition = LocalDefinition(locals, values);
                ast->statements.push_back(definition);
            }
        }

        // Run the parsing function for the current operator.
        const auto result = TABLE[op](state, ast, i, function);

        // Return on error.
        if(result != Status::OK)
        {

#ifndef NDEBUG
            printf(
                "Parser error at line %d: %u (%s), instruction 0x%08X (%s) at PC %d.\n",
                function.line_defined,
                static_cast<unsigned>(result),
                STATUS_TO_STR[result].c_str(),
                i,
                OP_TO_STR[OP(i)].c_str(),
                state.PC);

            state.print();
#endif

            return result;
        }

        // Conditional statements are handled implicitly through the jump offset
        // (S register) of the instruction. The offset has to be checked if the parser
        // is currently inside a condition block.
        if(state.PC == ast->context.jump_offset)
        {
            // Inline or comparison for an assignment (x = x or y)
            if(ast->context.is_or_block)
            {
                auto left = std::get<Expression>(state.stack.back());
                state.stack.pop_back();

                auto& operation =
                    std::get<AstOperation>(std::get<Expression>(state.stack.back()));
                operation.ex.push_back(left);

                ast->context.is_or_block = false;
            }
            // Handle the end of a condition block if the PC is right.
            while(ast->context.is_condition && state.PC >= ast->context.jump_offset)
            {
                auto& condition = std::get<Condition>(ast->parent->statements.back());

                // Create an else block if the last jump operator was a JMP
                if(ast->context.is_jmp_block)
                {
                    const auto operation = AstOperation("", {});
                    const auto block     = ConditionBlock(operation, {});
                    condition.blocks.push_back(block);
                }

                condition.blocks.back().statements = ast->statements;
                ast->statements.clear();

                ast->context.is_condition = false;
                exit_block(state, ast);
            }
        }
        state.PC++;
    }

    return Status::OK;
}
