#include "define.h"

void ForLoop::print()
{
    printf("for %s , %s , %s do\n", 
            begin.c_str(), end.c_str(), increment.c_str());

    for(auto statement : statements)
    {
        std::visit([](auto && s) { s.print(); }, statement);
        printf("\n");
    }

    printf("end");
}

void ForInLoop::print()
{
    printf("for %s in %s do\n", left.c_str(), right.c_str());

    for(auto statement : statements)
    {
        std::visit([](auto && s) { s.print(); }, statement);
        printf("\n");
    }

    printf("end");
}

void WhileLoop::print()
{
    printf("while %s do\n", condition.c_str());

    for(auto statement : statements)
    {
        std::visit([](auto && s) { s.print(); }, statement);
        printf("\n");
    }

    printf("end");
}

namespace
{

    // Block management

    void enter_block(Ast* & ast, const Token& from, const Token& to)
    {
        auto * body = new Ast();
        body->parent = ast;

        ast->body = body;
        ast = body;
    }

    void exit_block(Ast* & ast)
    {
        ast = ast->parent;
    }


    // Helpers

    void empty(Ast* & ast, const Token& from, const Token& to) {}


    // Stack modification

    void push_global(Ast* & ast, const Token& from, const Token& to)
    {
        const auto name = from.function->globals[B(from.instruction)];
        ast->stack.push_back(name);
    }

    void push_local(Ast* & ast, const Token& from, const Token& to)
    {
        const auto name = from.function->locals[B(from.instruction)];
        ast->stack.push_back(name);
    }

    void push_int(Ast* & ast, const Token& from, const Token& to)
    {
        const auto val = S(from.instruction);
        ast->stack.push_back(std::to_string(val));
    }

    void push_num(Ast* & ast, const Token& from, const Token& to)
    {
        const auto val = from.function->numbers[B(from.instruction)];
        ast->stack.push_back(std::to_string(val));
    }

    void push_string(Ast* & ast, const Token& from, const Token& to)
    {
        auto str = std::string("\"");
        str.append(from.function->globals[B(from.instruction)]);
        str.append("\"");
        ast->stack.push_back(str);
    }

    void push_list(Ast* & ast, const Token& from, const Token& to)
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

    void push_map(Ast* & ast, const Token& from, const Token& to)
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

    void make_assignment(Ast* & ast, const Token& from, const Token& to)
    {
        Assignment ass;
        ass.left = from.function->globals[B(from.instruction)];
        ass.right = ast->stack.back();
        ast->stack.pop_back();

        ast->statements.push_back(ass);
    }


    // Call

    void make_call(Ast* & ast, const Token& from, const Token& to)
    {
        Call call;
        call.name = *ast->stack.begin();
        call.arguments = {ast->stack.begin() + 1, ast->stack.end()};

        ast->statements.push_back(call);
        ast->stack.clear();
    }


    // For loop

    void make_for_loop(Ast* & ast, const Token& from, const Token& to)
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

    void make_for_in_loop(Ast* & ast, const Token& from, const Token& to)
    {
        exit_block(ast);

        ForInLoop loop;

        loop.right = ast->stack.back();
        ast->stack.pop_back();

        loop.statements = ast->body->statements;

        ast->statements.push_back(loop);
    }


    // While loop

    void make_while_loop(Ast* & ast, const Token& from, const Token& to)
    {
        WhileLoop loop;

        loop.condition = ast->stack.back();
        ast->stack.pop_back();

        loop.statements = ast->body->statements;

        ast->statements.push_back(loop);
    }


    // Transitions

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
        // While loop
        {Operator::JMP,         &empty},
        {Operator::JMPF,        &empty},
    };
}


void run_state_machine(Ast* & ast, const TokenList& tokens)
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

        //printf("DEBUG: %d\n", from);

        auto action = TABLE[from];
        action(ast, *curr, *next);

        curr = next;
    }
    while(curr != tokens.end());
        printf("DEBUG: End\n");
}

