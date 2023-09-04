#include "parser/parser.hpp"

// Block management

void enter_block(Ast*& ast)
{
    auto* body   = new Ast();
    body->parent = ast;

    ast->body = body;
    ast       = body;
}

void enter_block(Ast*& ast, const Token& from, const Token& to)
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

void empty(Ast*& ast, const Token& from, const Token& to)
{
}

// Stack modification

void push_global(Ast*& ast, const Token& from, const Token& to)
{
    const auto name = from.function->globals[B(from.instruction)];
    ast->stack.push_back(name);
}

void push_local(Ast*& ast, const Token& from, const Token& to)
{
    const auto name = from.function->locals[B(from.instruction)];
    ast->stack.push_back(name);
}

void push_int(Ast*& ast, const Token& from, const Token& to)
{
    const auto val = S(from.instruction);
    ast->stack.push_back(std::to_string(val));
}

void push_num(Ast*& ast, const Token& from, const Token& to)
{
    const auto val = from.function->numbers[B(from.instruction)];
    ast->stack.push_back(std::to_string(val));
}

void push_string(Ast*& ast, const Token& from, const Token& to)
{
    auto str = std::string("\"");
    str.append(from.function->globals[B(from.instruction)]);
    str.append("\"");
    ast->stack.push_back(str);
}

void push_list(Ast*& ast, const Token& from, const Token& to)
{
    auto list = std::string("{");

    auto it = ast->stack.begin();
    while(it != ast->stack.end())
    {
        list.append((*it).c_str());

        if(it != ast->stack.end() - 1)
            list.append(", ");

        it++;
    }
    list.append("}");

    ast->stack.clear();
    ast->stack.push_back(list);
}

void push_map(Ast*& ast, const Token& from, const Token& to)
{
    auto list = std::string("{");

    auto it = ast->stack.begin();
    while(it != ast->stack.end())
    {
        list.append((*it).c_str());
        list.append(" = ");
        it++;
        list.append((*it).c_str());

        if(it != ast->stack.end() - 1)
            list.append(", ");

        it++;
    }
    list.append("}");

    ast->stack.clear();
    ast->stack.push_back(list);
}

// Assignment

void make_assignment(Ast*& ast, const Token& from, const Token& to)
{
    Assignment ass;
    ass.left  = from.function->globals[B(from.instruction)];
    ass.right = ast->stack.back();
    ast->stack.pop_back();

    ast->statements.push_back(ass);
}

// Call

void make_call(Ast*& ast, const Token& from, const Token& to)
{
    Call call;
    call.name      = *ast->stack.begin();
    call.arguments = {ast->stack.begin() + 1, ast->stack.end()};

    ast->statements.push_back(call);
    ast->stack.clear();
}

// For loop

void make_for_loop(Ast*& ast, const Token& from, const Token& to)
{
    exit_block(ast);

    ForLoop loop;

    loop.increment = ast->stack.back();
    ast->stack.pop_back();

    loop.end = ast->stack.back();
    ast->stack.pop_back();

    loop.begin = ast->stack.back();
    ast->stack.pop_back();

    loop.statements = ast->body->statements;

    ast->statements.push_back(loop);
}

// For in loop

void make_for_in_loop(Ast*& ast, const Token& from, const Token& to)
{
    exit_block(ast);

    ForInLoop loop;

    loop.right = ast->stack.back();
    ast->stack.pop_back();

    loop.statements = ast->body->statements;

    ast->statements.push_back(loop);
}

// While loop

void make_while_loop(Ast*& ast, const Token& from, const Token& to)
{
    WhileLoop loop;

    loop.condition = ast->stack.back();
    ast->stack.pop_back();

    loop.statements = ast->body->statements;

    ast->statements.push_back(loop);
}

// Condition

void make_condition(Ast*& ast, const Token& from, const Token& to)
{
    Condition   condition;
    std::string left;
    std::string middle;
    std::string right;

    Operator op = OP(from.instruction);

    if(op >= Operator::JMPNE && op <= Operator::JMPGE)
    {
        right = ast->stack.back();
        ast->stack.pop_back();

        left = ast->stack.back();
        ast->stack.pop_back();

        switch(op)
        {
        case Operator::JMPNE:
            middle = " ~= ";
            break;
        case Operator::JMPEQ:
            middle = " == ";
            break;
        case Operator::JMPLT:
            middle = " < ";
            break;
        case Operator::JMPLE:
            middle = " <= ";
            break;
        case Operator::JMPGT:
            middle = " > ";
            break;
        case Operator::JMPGE:
            middle = " >= ";
            break;
        default:
            printf("OP %d not convered for conditions\n", op);
        }
    }
    else if(op >= Operator::JMPT && op <= Operator::JMPONF)
    {
        switch(op)
        {
        case Operator::JMPT:
            middle = " ~= nil";
            break;
        case Operator::JMPF:
            middle = " == nil";
            break;
        case Operator::JMPONT:
            middle = " ~= nil ";
            break;
        case Operator::JMPONF:
            middle = " == nil ";
            break;
        default:
            printf("OP %d not convered for conditions\n", op);
        }
    }

    condition.condition = left.append(middle).append(right);

    ast->statements.push_back(condition);

    enter_block(ast);
}

void end_condition(Ast*& ast, const Token& from, const Token& to)
{
    exit_block(ast);

    Condition& condition = std::get<Condition>(ast->statements.back());
    condition.statements = ast->body->statements;
}

TokenList parse_bytecode(const Function& function)
{
    auto tokens = TokenList();
    tokens.reserve(function.instructions.size());

    for(const auto& instruction : function.instructions)
    {
        Token token;
        token.instruction = instruction;
        token.function    = const_cast<Function*>(&function);
        tokens.push_back(token);
        debug_instruction(instruction);
    }

    for(const auto func : function.functions)
    {
        parse_bytecode(func);
    }

    return tokens;
}

// clang-format off
auto TABLE = TransitionTable
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
};

// clang-format on
void run_state_machine(Ast*& ast, const TokenList& tokens)
{
    auto curr = tokens.begin();

    if(tokens.size() < 2)
    {
        printf("DEBUG: Too few tokens\n");
        return;
    }

    printf("DEBUG: Go\n");
    do
    {
        auto next = curr + 1;
        auto from = Operator(OP((*curr).instruction));

        if(TABLE.count(from) == 0)
        {
            printf("DEBUG: No state %d in table\n", from);
            return;
        }

        // printf("DEBUG: %d\n", from);

        auto action = TABLE[from];
        action(ast, *curr, *next);

        curr = next;
    } while(curr != tokens.end());
    printf("DEBUG: End\n");
}
