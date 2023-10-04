#include "ast/ast.hpp"

void print_ast(const Ast& ast)
{
    print_statements(ast.statements, 0);
}

void print_indent(const int indent)
{
    printf("%*s", indent * 4, "");
}

void print_statements(const std::vector<Statement>& statements, const int indent)
{
    for(auto statement : statements)
    {
        print_indent(indent);
        std::visit([indent](auto&& s) { print(s, indent); }, statement);
        printf("\n");
    }
}

void print(const Assignment& assignment, const int indent)
{
    print_indent(indent);
    printf("%s = %s", assignment.left.c_str(), assignment.right.c_str());
}

void print(const Call& call, const int indent)
{
    print_indent(indent);
    printf("%s(", call.name.c_str());

    auto it = call.arguments.begin();
    while(it != call.arguments.end())
    {
        printf("%s", (*it).c_str());

        if(it != call.arguments.end() - 1)
            printf(", ");

        it++;
    }
    printf(")");
}

void print(const ForLoop& loop, const int indent)
{
    print_indent(indent);
    printf("for %s , %s , %s do\n", loop.begin.c_str(), loop.end.c_str(), loop.increment.c_str());

    print_statements(loop.statements, indent + 1);

    print_indent(indent);
    printf("end");
}

void print(const ForInLoop& loop, const int indent)
{
    print_indent(indent);
    printf("for %s in %s do\n", loop.left.c_str(), loop.right.c_str());

    print_statements(loop.statements, indent + 1);

    print_indent(indent);
    printf("end");
}

void print(const WhileLoop& loop, const int indent)
{
    print_indent(indent);
    printf("while %s do\n", loop.condition.c_str());

    print_statements(loop.statements, indent + 1);

    print_indent(indent);
    printf("end");
}

void print(const Condition& condition, const int indent)
{
    print_indent(indent);
    printf("if %s then\n", condition.condition.c_str());

    print_statements(condition.statements, indent + 1);

    print_indent(indent);
    printf("end");
}

void print(const Closure& closure, const int indent)
{
    print_indent(indent);
    printf("%s = function()\n", closure.name.c_str());

    print_statements(closure.statements, indent + 1);

    print_indent(indent);
    printf("end");
}
