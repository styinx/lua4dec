#include "parser/parser.hpp"

// Block management

void enter_block(Ast*& ast)
{
    auto* body   = new Ast();
    body->parent = ast;

    ast->body = body;
    ast       = body;
}

void enter_block(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    auto* body   = new Ast();
    body->parent = ast;

    ast->body = body;
    ast       = body;
}

void exit_block(Ast*& ast)
{
    if(ast->parent)
        ast = ast->parent;
}

// Helpers

void empty(Ast*&, const Instruction&, const Function&)
{
}

// Stack modification
void push_nil(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    ast->stack.push_back(Identifier("nil"));
}

void pop(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    ast->stack.pop_back();
}

void push_global(Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto name = function.globals[U(instruction)];
    ast->stack.push_back(Identifier(name));
}

void push_dotted(Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto name   = function.globals[U(instruction)];
    const auto parent = std::get<Identifier>(std::get<Expression>(ast->stack.back()));
    ast->stack.pop_back();
    ast->stack.push_back(Identifier(parent.name + "." + name));
}

void push_indexed(Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto name   = function.locals[U(instruction)];
    const auto parent = std::get<Identifier>(std::get<Expression>(ast->stack.back()));
    ast->stack.pop_back();
    ast->stack.push_back(Identifier(parent.name + "[" + name + "]"));
}

void push_self(Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto name   = function.globals[U(instruction)];
    const auto parent = std::get<Identifier>(std::get<Expression>(ast->stack.back()));
    ast->stack.push_back(Identifier(parent.name + "." + name));
}

void push_local(Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto name = function.locals[U(instruction)];
    ast->stack.push_back(Identifier(name));
}

void push_int(Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    const auto value = S(instruction);
    ast->stack.push_back(AstInt(value));
}

void push_num(Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto value = function.numbers[U(instruction)];
    ast->stack.push_back(AstNumber(value));
}

void push_string(Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto value = function.globals[U(instruction)];
    ast->stack.push_back(AstString(value));
}

void push_list(Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    Collection<Expression> list;
    for(unsigned i = 0; i < B(instruction); ++i)
    {
        list.push_back(std::get<Expression>(ast->stack.back()));
        ast->stack.pop_back();
    }

    std::reverse(list.begin(), list.end());

    ast->stack.push_back(AstList(list));
}

void push_map(Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    Collection<std::pair<Expression, Expression>> list;
    for(unsigned i = 0; i < U(instruction); ++i)
    {
        const auto value = std::get<Expression>(ast->stack.back());
        ast->stack.pop_back();

        const auto key = std::get<Expression>(ast->stack.back());
        ast->stack.pop_back();

        list.push_back(std::make_pair(key, value));
    }

    std::reverse(list.begin(), list.end());

    ast->stack.push_back(AstMap(list));
}

// Operations
void add(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto right = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    const auto left = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    ast->stack.push_back(AstOperation("+", {left, right}));
}

void addi(Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    const auto right = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    const auto left = AstNumber(S(instruction));
    ast->stack.push_back(AstOperation("+", {left, right}));
}

void sub(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto right = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    const auto left = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    ast->stack.push_back(AstOperation("-", {left, right}));
}

void mult(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto right = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    const auto left = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    ast->stack.push_back(AstOperation("*", {left, right}));
}

void div(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto right = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    const auto left = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    ast->stack.push_back(AstOperation("/", {left, right}));
}

void pow(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto right = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    const auto left = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    ast->stack.push_back(AstOperation("^", {left, right}));
}

void concat(Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    Collection<Expression> ex;
    for(unsigned i = 0; i < U(instruction); ++i)
    {
        ex.push_back(std::get<Expression>(ast->stack.back()));
        ast->stack.pop_back();
    }
    ast->stack.push_back(AstOperation("..", ex));
}

void minus(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto right = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    ast->stack.push_back(AstOperation("-", {right}));
}

void not(Ast * &ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto right = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    ast->stack.push_back(AstOperation("not", {right}));
}

// Assignment

void make_assignment(Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto left  = Identifier(function.globals[B(instruction)]);
    const auto right = std::get<Expression>(ast->stack.back());
    Assignment ass(left, right);
    ast->stack.pop_back();

    ast->statements.push_back(ass);
}

void make_local_assignment(Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto left  = Identifier(function.locals[B(instruction)]);
    const auto right = std::get<Expression>(ast->stack.back());
    Assignment ass(left, right);
    ast->stack.pop_back();

    ast->statements.push_back(ass);
}

// Call

void make_call(Ast*& ast, const Instruction& instruction, const Function& function)
{
    // A represents the number of elements to keep on the stack. All popped elements
    // make up the name and the argument of the function.

    Collection<Expression> args;
    const auto             keep_stack_elements = A(instruction);
    while(ast->stack.size() > keep_stack_elements + 1)
    {
        args.push_back(std::get<Expression>(ast->stack.back()));
        ast->stack.pop_back();
    }

    auto name = std::get<Identifier>(std::get<Expression>(ast->stack.back()));
    ast->stack.pop_back();

    std::reverse(args.begin(), args.end());

    ast->statements.push_back(Call(name, args));
}

void make_tail_call(Ast*& ast, const Instruction& instruction, const Function& function)
{
    // A represents the number of elements to keep on the stack. All popped elements
    // make up the name and the argument of the function.

    Collection<Expression> args;
    const auto             keep_stack_elements = A(instruction);
    while(ast->stack.size() > keep_stack_elements + 1)
    {
        args.push_back(std::get<Expression>(ast->stack.back()));
        ast->stack.pop_back();
    }

    auto name = std::get<Identifier>(std::get<Expression>(ast->stack.back()));
    ast->stack.pop_back();

    std::reverse(args.begin(), args.end());

    ast->statements.push_back(TailCall(name, args));
}

// For loop

void make_for_loop(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    exit_block(ast);

    const auto increment = std::get<AstInt>(std::get<Expression>(ast->stack.back()));
    ast->stack.pop_back();

    const auto end = std::get<AstInt>(std::get<Expression>(ast->stack.back()));
    ast->stack.pop_back();

    const auto begin = std::get<AstInt>(std::get<Expression>(ast->stack.back()));
    ast->stack.pop_back();

    const ForLoop loop(begin, end, increment, ast->body->statements);
    ast->statements.push_back(loop);
}

// For in loop

void make_for_in_loop(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    exit_block(ast);

    ForInLoop loop;

    // loop.right = std::get<Identifier>(std::get<Expression>(ast->stack.back())).name;
    ast->stack.pop_back();

    loop.statements = ast->body->statements;

    ast->statements.push_back(loop);
}

// While loop

void make_while_loop(Ast*& /*ast*/, const Instruction& /*instruction*/, const Function& /*function*/)
{
    // TODO: while loop does not really exist in byte code
}

// Condition

void make_binary_condition(Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    const auto op = OP(instruction);

    if(op < Operator::JMPNE || op > Operator::JMPGE)
    {
        printf("Invalid binary operator OP %d (%s) \n", (int)op, OP_TO_STR[op].c_str());
    }

    auto right = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();

    auto left = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();

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
    default:
        printf("OP %d not covered for conditions\n", (int)op);
    }

    ast->statements.push_back(
        Condition(AstOperation(comparison, {left, right}), Collection<Statement>{}));

    enter_block(ast);
}

void make_unary_condition(Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    const auto op = OP(instruction);

    if(op < Operator::JMPT || op > Operator::JMPONF)
    {
        printf("Invalid unary operator OP %d (%s) \n", (int)op, OP_TO_STR[op].c_str());
    }

    std::string comparison;

    switch(op)
    {
    case Operator::JMPT:
    case Operator::JMPONT:
        comparison = "~=";
        break;
    case Operator::JMPF:
    case Operator::JMPONF:
        comparison = "==";
        break;
    default:
        printf("OP %d not covered for conditions\n", (int)op);
    }

    const auto left = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();

    ast->statements.push_back(
        Condition(AstOperation(comparison, {left, Identifier("nil")}), Collection<Statement>{}));

    enter_block(ast);
}

void end_condition(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    exit_block(ast);

    Condition& condition = std::get<Condition>(ast->statements.back());
    condition.statements = ast->body->statements;
}

void make_closure(Ast*& ast, const Instruction& instruction, const Function& function)
{
    enter_block(ast);

    parse_function(ast, function.functions[A(instruction)]);

    // Leftover elements on the stack make up the local variables.
    const auto                  locals = function.functions[A(instruction)].locals;
    Collection<LocalAssignment> args;
    unsigned                    i = 0;
    for(const auto& e : ast->stack)
    {
        const auto ass = LocalAssignment(Identifier(locals[i++]), std::get<Expression>(e));
        args.push_back(ass);
    }

    exit_block(ast);

    ast->stack.push_back(Closure(ast->body->statements, args));
}

// Public functions

// clang-format off
auto TABLE = ActionTable
{
    {Operator::END,         &empty},
    {Operator::RETURN,      &empty}, // TODO
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
    {Operator::GETTABLE,    &empty}, // TODO
    {Operator::GETDOTTED,   &push_dotted},
    {Operator::GETINDEXED,  &push_indexed},
    {Operator::PUSHSELF,    &push_self},
    {Operator::CREATETABLE, &empty}, // TODO
    {Operator::SETLIST,     &push_list},
    {Operator::SETMAP,      &push_map},
    {Operator::PUSHNILJMP,  &empty}, // TODO
    // Operations
    {Operator::ADD,         &add},
    {Operator::ADDI,        &addi},
    {Operator::SUB,         &sub},
    {Operator::MULT,        &mult},
    {Operator::DIV,         &div},
    {Operator::POW,         &pow},
    {Operator::CONCAT,      &concat},
    {Operator::MINUS,       &minus},
    {Operator::NOT,         &not},
    // Call
    {Operator::CALL,        &make_call},
    {Operator::TAILCALL,    &make_tail_call},
    // Assignment
    {Operator::SETLOCAL,    &make_local_assignment},
    {Operator::SETGLOBAL,   &make_assignment},
    {Operator::SETTABLE,    &empty},  // TODO
    // For loop
    {Operator::FORPREP,     &enter_block},
    {Operator::FORLOOP,     &make_for_loop},
    // For in loop
    {Operator::LFORPREP,    &enter_block},
    {Operator::LFORLOOP,    &make_for_in_loop},
    // Conditions
    {Operator::JMPNE,       &make_binary_condition},
    {Operator::JMPEQ,       &make_binary_condition},
    {Operator::JMPLT,       &make_binary_condition},
    {Operator::JMPLE,       &make_binary_condition},
    {Operator::JMPGT,       &make_binary_condition},
    {Operator::JMPGE,       &make_binary_condition},
    {Operator::JMPT,        &make_unary_condition},
    {Operator::JMPF,        &make_unary_condition},
    {Operator::JMPONT,      &make_unary_condition},
    {Operator::JMPONF,      &make_unary_condition},
    {Operator::JMP,         &end_condition},
    // Closure
    {Operator::CLOSURE,     &make_closure},
};

// clang-format on

void parse_function(Ast*& ast, const Function& function)
{
    // TODO: Add local variables for this scope

    for(const auto& i : function.instructions)
    {
        auto op = Operator(OP(i));

        if(TABLE.count(op) == 0)
        {
            printf("DEBUG: No action for %d (%s) in table\n", (int)op, OP_TO_STR[op].c_str());
            return;
        }

        TABLE[op](ast, i, function);
    }
}
