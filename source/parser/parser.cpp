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
void push_global(Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto name = function.globals[B(instruction)];
    ast->stack.push_back(Identifier(name));
}

void push_local(Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto name = function.locals[B(instruction)];
    ast->stack.push_back(Identifier(name));
}

void push_int(Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    const auto value = S(instruction);
    ast->stack.push_back(AstInt(value));
}

void push_num(Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto value = function.numbers[B(instruction)];
    ast->stack.push_back(AstNumber(value));
}

void push_string(Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto value = function.globals[B(instruction)];
    ast->stack.push_back(AstString(value));
}

void push_list(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    Collection<Expression> list;
    for(const auto& e : ast->stack)
    {
        list.push_back(std::get<Expression>(e));
    }

    ast->stack.clear();
    ast->stack.push_back(AstList(list));
}

void push_map(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    Collection<std::pair<Expression, Expression>> list;
    for(auto it = ast->stack.begin(); it != ast->stack.end();)
    {
        const auto first = std::get<Expression>(*it);
        it++;

        const auto second = std::get<Expression>(*it);
        it++;

        list.push_back(std::make_pair(first, second));
    }

    ast->stack.clear();
    ast->stack.push_back(AstMap(list));
}

// Assignment

void make_assignment(Ast*& ast, const Instruction& instruction, const Function& function)
{
    auto       left  = Identifier(function.globals[B(instruction)]);
    auto       right = std::get<Expression>(ast->stack.back());
    Assignment ass(left, right);
    ast->stack.pop_back();

    ast->statements.push_back(ass);
}

// Call

void make_call(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    // TODO: We can not be sure that the lowest element on the stack is the function name ...
    auto ex = std::get<Expression>(*ast->stack.begin());
    // auto name = std::get<Identifier>(ex);
    Call call(Identifier("call"), Collection<Expression>{});
    // call.arguments = {ast->stack.begin() + 1, ast->stack.end()};

    ast->statements.push_back(call);
    ast->stack.clear();
}

// For loop

void make_for_loop(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    exit_block(ast);

    auto increment = std::get<AstInt>(std::get<Expression>(ast->stack.back()));
    ast->stack.pop_back();

    auto end = std::get<AstInt>(std::get<Expression>(ast->stack.back()));
    ast->stack.pop_back();

    auto begin = std::get<AstInt>(std::get<Expression>(ast->stack.back()));
    ast->stack.pop_back();

    ForLoop loop(begin, end, increment, ast->body->statements);
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

void make_condition(Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    std::string operation;

    const auto op = OP(instruction);

    if(op >= Operator::JMPNE && op <= Operator::JMPGE)
    {
        auto right = std::get<Expression>(ast->stack.back());
        ast->stack.pop_back();

        auto left = std::get<Expression>(ast->stack.back());
        ast->stack.pop_back();

        switch(op)
        {
        case Operator::JMPNE:
            operation = "==";
            break;
        case Operator::JMPEQ:
            operation = "~=";
            break;
        case Operator::JMPLT:
            operation = ">=";
            break;
        case Operator::JMPLE:
            operation = ">";
            break;
        case Operator::JMPGT:
            operation = "<=";
            break;
        case Operator::JMPGE:
            operation = "<";
            break;
        default:
            printf("OP %d not covered for conditions\n", (int)op);
        }

        ast->statements.push_back(
            Condition(AstOperation(operation, {left, right}), Collection<Statement>{}));
    }
    else if(op >= Operator::JMPT && op <= Operator::JMPONF)
    {
        switch(op)
        {
        case Operator::JMPT:
            operation = "~= nil";
            break;
        case Operator::JMPF:
            operation = "== nil";
            break;
        case Operator::JMPONT:
            operation = "~= nil";
            break;
        case Operator::JMPONF:
            operation = "== nil";
            break;
        default:
            printf("OP %d not covered for conditions\n", (int)op);
        }

        Expression e1 = AstNumber(1);
        Expression e2 = AstNumber(1);
        ast->statements.push_back(
            Condition(AstOperation(operation, {e1, e2}), Collection<Statement>{}));
    }
    else
    {
        assert(false);
    }

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
    exit_block(ast);

    ast->stack.push_back(Closure(ast->body->statements));
}

// Public functions

// clang-format off
auto TABLE = ActionTable
{
    {Operator::END,         &empty},
    // Stack modification
    {Operator::PUSHINT,     &push_int},
    {Operator::PUSHSTRING,  &push_string},
    {Operator::PUSHNUM,     &push_num},
    {Operator::GETGLOBAL,   &push_global},
    {Operator::GETLOCAL,    &push_local},
    //TODO: table uses number of elements on stack
    {Operator::CREATETABLE, &empty},
    {Operator::SETLIST,     &push_list},
    {Operator::SETMAP,      &push_map},
    // Call
    {Operator::CALL,        &make_call},
    // Assignment
    {Operator::SETGLOBAL,   &make_assignment},
    // For loop
    {Operator::FORPREP,     &enter_block},
    {Operator::FORLOOP,     &make_for_loop},
    // For in loop
    {Operator::LFORPREP,    &enter_block},
    {Operator::LFORLOOP,    &make_for_in_loop},
    // Conditions
    {Operator::JMPNE,       &make_condition},
    {Operator::JMPEQ,       &make_condition},
    {Operator::JMPLT,       &make_condition},
    {Operator::JMPLE,       &make_condition},
    {Operator::JMPGT,       &make_condition},
    {Operator::JMPGE,       &make_condition},
    {Operator::JMPT,        &make_condition},
    {Operator::JMPF,        &make_condition},
    {Operator::JMPONT,      &make_condition},
    {Operator::JMPONF,      &make_condition},
    {Operator::JMP,         &end_condition},
    // Closure
    {Operator::CLOSURE,     &make_closure},
};

// clang-format on

void parse_function(Ast*& ast, const Function& function)
{
    for(const auto& i : function.instructions)
    {
        auto op = Operator(OP(i));

        if(TABLE.count(op) == 0)
        {
            printf("DEBUG: No action for %d in table\n", (int)op);
            return;
        }

        TABLE[op](ast, i, function);
    }
}
