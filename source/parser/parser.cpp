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

void enter_block(State&, Ast*&, const Instruction&, const Function&);
void exit_block(State&, Ast*&);
void empty(State&, Ast*&, const Instruction&, const Function&);
void push_nil(State&, Ast*&, const Instruction&, const Function&);
void push_int(State&, Ast*&, const Instruction&, const Function&);
void push_string(State&, Ast*&, const Instruction&, const Function&);
void push_num(State&, Ast*&, const Instruction&, const Function&);
void push_local(State&, Ast*&, const Instruction&, const Function&);
void push_global(State&, Ast*&, const Instruction&, const Function&);
void push_table_element(State&, Ast*&, const Instruction&, const Function&);
void push_dotted(State&, Ast*&, const Instruction&, const Function&);
void push_indexed(State&, Ast*&, const Instruction&, const Function&);
void push_self(State&, Ast*&, const Instruction&, const Function&);
void push_table(State&, Ast*&, const Instruction&, const Function&);
void push_list(State&, Ast*&, const Instruction&, const Function&);
void push_map(State&, Ast*&, const Instruction&, const Function&);
void pop(State&, Ast*&, const Instruction&, const Function&);
void make_add(State&, Ast*&, const Instruction&, const Function&);
void make_addi(State&, Ast*&, const Instruction&, const Function&);
void make_sub(State&, Ast*&, const Instruction&, const Function&);
void make_mult(State&, Ast*&, const Instruction&, const Function&);
void make_div(State&, Ast*&, const Instruction&, const Function&);
void make_pow(State&, Ast*&, const Instruction&, const Function&);
void make_concat(State&, Ast*&, const Instruction&, const Function&);
void make_minus(State&, Ast*&, const Instruction&, const Function&);
void make_not(State&, Ast*&, const Instruction&, const Function&);
void make_call(State&, Ast*&, const Instruction&, const Function&);
void make_tail_call(State&, Ast*&, const Instruction&, const Function&);
void make_local_assignment(State&, Ast*&, const Instruction&, const Function&);
void make_return(State&, Ast*&, const Instruction&, const Function&);
void make_assignment(State&, Ast*&, const Instruction&, const Function&);
void make_table_assignment(State&, Ast*&, const Instruction&, const Function&);
void enter_for_in_loop(State&, Ast*&, const Instruction&, const Function&);
void enter_for_loop(State&, Ast*&, const Instruction&, const Function&);
void exit_for_loop(State&, Ast*&, const Instruction&, const Function&);
void exit_for_in_loop(State&, Ast*&, const Instruction&, const Function&);
void make_or(State&, Ast*&, const Instruction&, const Function&);
void make_condition(State&, Ast*&, const Instruction&, const Function&);
void end_condition(State&, Ast*&, const Instruction&, const Function&);
void make_closure(State&, Ast*&, const Instruction&, const Function&);

// clang-format off
auto TABLE = ActionTable
{
    {Operator::END,         &empty},
    {Operator::RETURN,      &make_return},
    // Stack modification
    {Operator::PUSHNIL,     &push_nil},
    {Operator::POP,         &pop},
    {Operator::PUSHINT,     &push_int},
    {Operator::PUSHSTRING,  &push_string},
    {Operator::PUSHNUM,     &push_num},
    {Operator::PUSHNEGNUM,  &push_num},
    {Operator::PUSHUPVALUE, &empty}, // TODO
    {Operator::GETLOCAL,    &push_local},
    {Operator::GETGLOBAL,   &push_global},
    {Operator::GETTABLE,    &push_table_element},
    {Operator::GETDOTTED,   &push_dotted},
    {Operator::GETINDEXED,  &push_indexed},
    {Operator::PUSHSELF,    &push_self},
    {Operator::CREATETABLE, &push_table},
    {Operator::SETLIST,     &push_list},
    {Operator::SETMAP,      &push_map},
    {Operator::PUSHNILJMP,  &empty},
    // Operations
    {Operator::ADD,         &make_add},
    {Operator::ADDI,        &make_addi},
    {Operator::SUB,         &make_sub},
    {Operator::MULT,        &make_mult},
    {Operator::DIV,         &make_div},
    {Operator::POW,         &make_pow},
    {Operator::CONCAT,      &make_concat},
    {Operator::MINUS,       &make_minus},
    {Operator::NOT,         &make_not},
    // Call
    {Operator::CALL,        &make_call},
    {Operator::TAILCALL,    &make_tail_call},
    // Assignment
    {Operator::SETLOCAL,    &make_local_assignment},
    {Operator::SETGLOBAL,   &make_assignment},
    {Operator::SETTABLE,    &make_table_assignment},
    // For loop
    {Operator::FORPREP,     &enter_for_loop},
    {Operator::FORLOOP,     &exit_for_loop},
    // For in loop
    {Operator::LFORPREP,    &enter_for_in_loop},
    {Operator::LFORLOOP,    &exit_for_in_loop},
    // Conditions
    {Operator::JMPNE,       &make_condition},
    {Operator::JMPEQ,       &make_condition},
    {Operator::JMPLT,       &make_condition},
    {Operator::JMPLE,       &make_condition},
    {Operator::JMPGT,       &make_condition},
    {Operator::JMPGE,       &make_condition},
    {Operator::JMPT,        &make_condition},
    {Operator::JMPF,        &make_condition},
    {Operator::JMPONT,      &make_or},
    {Operator::JMPONF,      &make_condition},
    {Operator::JMP,         &end_condition},
    // Closure
    {Operator::CLOSURE,     &make_closure},
};

// clang-format on

// Block management

void enter_block(Ast*& ast)
{
    auto* child   = new Ast();
    child->parent = ast;

    ast->child = child;
    ast        = child;
}

void enter_block(State& state, Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    auto* child   = new Ast();
    child->parent = ast;

    ast->child = child;
    ast        = child;
}

void exit_block(Ast*& ast)
{
    if(ast->parent)
        ast = ast->parent;
}

// Helpers

void empty(State&, Ast*&, const Instruction&, const Function&)
{
}

// Stack modification

void push_nil(State& state, Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    state.stack.push_back(Identifier("nil"));
}

void pop(State& state, Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    auto elements = U(instruction);
    while(elements > 0)
    {
        // TODO: Shouldn't have to be checked
        if(state.stack.size())
            state.stack.pop_back();
        elements--;
    }
}

void push_global(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto name = function.globals[U(instruction)];
    state.stack.push_back(Identifier(name));
}

void push_table_element(
    State& state,
    Ast*&  ast,
    const Instruction& /*instruction*/,
    const Function& /*function*/)
{
    const auto index = std::get<Identifier>(std::get<Expression>(state.stack.back()));
    state.stack.pop_back();

    const auto table = std::get<Identifier>(std::get<Expression>(state.stack.back()));
    state.stack.pop_back();

    state.stack.push_back(Identifier(table.name + "[" + index.name + "]"));
}

void push_dotted(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto name   = function.globals[U(instruction)];
    const auto parent = std::get<Identifier>(std::get<Expression>(state.stack.back()));
    state.stack.pop_back();
    state.stack.push_back(Identifier(parent.name + "." + name));
}

void push_indexed(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto name   = function.locals[U(instruction)].name;
    const auto parent = std::get<Identifier>(std::get<Expression>(state.stack.back()));
    state.stack.pop_back();
    state.stack.push_back(Identifier(parent.name + "[" + name + "]"));
}

void push_self(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto name   = function.globals[U(instruction)];
    const auto parent = std::get<Identifier>(std::get<Expression>(state.stack.back()));
    state.stack.push_back(Identifier(parent.name + "." + name));
}

void push_local(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    // TODO: This is a special hack for SWBF
    const auto pos  = U(instruction);
    auto       name = String("local" + std::to_string(pos));
    if(function.locals.size() > pos)
        name = function.locals[pos].name;
    state.stack.push_back(Identifier(name));
}

void push_int(State& state, Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    const auto value = S(instruction);
    state.stack.push_back(AstInt(value));
}

void push_num(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto value = function.numbers[U(instruction)];
    state.stack.push_back(AstNumber(value));
}

void push_string(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto value = function.globals[U(instruction)];
    state.stack.push_back(AstString(value));
}

void push_table(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
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
}

void push_list(State& state, Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    Vector<Expression> list;
    for(unsigned i = 0; i < B(instruction); ++i)
    {
        list.push_back(std::get<Expression>(state.stack.back()));
        state.stack.pop_back();
    }

    std::reverse(list.begin(), list.end());

    state.stack.pop_back();  // empty AstTable
    state.stack.push_back(AstList(list));
}

void push_map(State& state, Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    Vector<std::pair<Expression, Expression>> map;
    for(unsigned i = 0; i < U(instruction); ++i)
    {
        const auto value = std::get<Expression>(state.stack.back());
        state.stack.pop_back();

        const auto key = std::get<Expression>(state.stack.back());
        state.stack.pop_back();

        map.push_back(std::make_pair(key, value));
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
}

// Operations
void make_add(State& state, Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    state.stack.push_back(AstOperation("+", {left, right}));
}

void make_addi(State& state, Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    const auto right = AstNumber(S(instruction));
    state.stack.push_back(AstOperation("+", {right, left}));
}

void make_sub(State& state, Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    state.stack.push_back(AstOperation("-", {left, right}));
}

void make_mult(State& state, Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    state.stack.push_back(AstOperation("*", {left, right}));
}

void make_div(State& state, Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    state.stack.push_back(AstOperation("/", {left, right}));
}

void make_pow(State& state, Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    state.stack.push_back(AstOperation("^", {left, right}));
}

void make_concat(State& state, Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    Vector<Expression> ex;
    for(unsigned i = 0; i < U(instruction); ++i)
    {
        ex.push_back(std::get<Expression>(state.stack.back()));
        state.stack.pop_back();
    }
    state.stack.push_back(AstOperation("..", ex));
}

void make_minus(State& state, Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    state.stack.push_back(AstOperation("-", {right}));
}

void make_not(State& state, Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto right = std::get<Expression>(state.stack.back());
    state.stack.pop_back();
    state.stack.push_back(AstOperation("not", {right}));
}

// Return

void make_return(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
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
}

// Assignment

void make_assignment(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto left  = Identifier(function.globals[B(instruction)]);
    const auto right = std::get<Expression>(state.stack.back());
    Assignment ass(left, right);
    state.stack.pop_back();

    ast->statements.push_back(ass);
}

void make_local_assignment(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto left  = Identifier(function.locals[B(instruction)].name);
    const auto right = std::get<Expression>(state.stack.back());
    Assignment ass(left, right);
    state.stack.pop_back();

    ast->statements.push_back(ass);
}

void make_table_assignment(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    Vector<Expression> args;
    for(unsigned i = 0; i < B(instruction); ++i)
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
}

// Call

void make_call(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
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
        // TODO
        // exit(1);
        int i = 0;
        state.stack.push_back(caller);
    }
}

void make_tail_call(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
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
}

// For loops

void enter_for_loop(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
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
}

void enter_for_in_loop(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto key   = get_local_from_pc(function, state.PC, 1);
    const auto value = get_local_from_pc(function, state.PC, 2);

    const auto right = std::get<Identifier>(std::get<Expression>(state.stack.back())).name;
    state.stack.pop_back();

    ForInLoop loop(key, value, right, Vector<Statement>{});
    ast->statements.push_back(loop);

    enter_block(ast);
}

void exit_for_loop(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    auto& loop      = std::get<ForLoop>(ast->parent->statements.back());
    loop.statements = ast->statements;

    exit_block(ast);
}

void exit_for_in_loop(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    auto& loop      = std::get<ForInLoop>(ast->parent->statements.back());
    loop.statements = ast->statements;

    exit_block(ast);
}

// While loop

void make_while_loop(Ast*& /*ast*/, const Instruction& /*instruction*/, const Function& /*function*/)
{
    // TODO: while loop does not really exist in byte code
}

// Condition

void make_or(State& state, Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    auto left = std::get<Expression>(state.stack.back());
    state.stack.pop_back();

    AstOperation operation("or", {left});
    state.stack.push_back(operation);

    ast->context.is_or_block = true;
    ast->context.jump_offset = state.PC + S(instruction);
}

void make_condition(State& state, Ast*& ast, const Instruction& instruction, const Function& /*function*/)
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
}

void end_condition(State& state, Ast*& ast, const Instruction& instruction, const Function& /*function*/)
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
}
void make_closure(State& state, Ast*& ast, const Instruction& instruction, const Function& function)
{
    enter_block(ast);

    // Parse the closure body which requires a new state
    auto new_state = State();
    parse_function(new_state, ast, function.functions[A(instruction)]);

    const auto locals = function.functions[A(instruction)].locals;

    Vector<Identifier> arguments;
    for(const auto& local : locals)
    {
        if(local.start_pc == 0)
        {
            arguments.push_back(Identifier(local.name));
        }
    }

    exit_block(ast);

    state.stack.push_back(Closure(ast->child->statements, arguments));
}

// Public functions

void parse_function(State& state, Ast*& ast, const Function& function)
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
            return;
        }

        // Local variable assignment (may be more than one)
        if(state.PC > 0 && state.stack.size())
        {
            Vector<Identifier> locals;
            for(const auto& local : function.locals)
            {
                if(local.start_pc == state.PC)
                {
                    locals.push_back(Identifier(local.name));
                }
            }

            if(locals.size())
            {
                const auto value = std::get<Expression>(state.stack.back());
                state.stack.pop_back();
                ast->statements.push_back(LocalAssignment(locals, value));

                // Locals need to be pushed onto the stack because of the offset of the
                // CALL operator.
                for(const auto& local : locals)
                    state.stack.push_back(local);
            }
        }

        TABLE[op](state, ast, i, function);

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
}
