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

// Expressions

void print(const Closure& closure, const int indent)
{
    printf("function()\n");

    print_statements(closure.statements, indent + 1);

    print_indent(indent);
    printf("end");
}

void print(const Identifier& identifier, const int /*indent*/)
{
    printf("%s", identifier.name.c_str());
}

void print(const AstInt& number, const int /*indent*/)
{
    printf("%ld", number.value);
}

void print(const AstList& list, const int indent)
{
    printf("{");
    for(const auto& el : list.elements)
    {
        std::visit([indent](auto&& e) { print(e, indent); }, el);

        if(&el != &list.elements.back())
            printf(", ");
    }
    printf("}");
}

void print(const AstMap& list, const int indent)
{
    printf("{");
    for(const auto& p : list.pairs)
    {
        std::visit([indent](auto&& e) { print(e, indent); }, p.first);
        printf(" = ");
        std::visit([indent](auto&& e) { print(e, indent); }, p.second);

        if(&p != &list.pairs.back())
            printf(", ");
    }
    printf("}");
}

void print(const AstNumber& number, const int /*indent*/)
{
    printf("%lf", number.value);
}

void print(const AstOperation& operation, const int indent)
{
    // TODO
    if(operation.ex.size() > 0)
        std::visit([indent](auto&& e) { print(e, indent); }, operation.ex[0]);

    printf("%s", operation.op.c_str());

    if(operation.ex.size() > 1)
        std::visit([indent](auto&& e) { print(e, indent); }, operation.ex[1]);
}

void print(const AstString& string, const int /*indent*/)
{
    printf("\"%s\"", string.value.c_str());
}

// Statements

void print(const Assignment& assignment, const int indent)
{
    print_indent(indent);
    printf("%s = ", assignment.left.name.c_str());
    std::visit([indent](auto&& e) { print(e, indent); }, assignment.right);
}

void print(const Call& call, const int indent)
{
    print_indent(indent);
    printf("%s(", call.name.name.c_str());

    auto it = call.arguments.begin();
    while(it != call.arguments.end())
    {
        std::visit([indent](auto&& e) { print(e, indent); }, *it);

        if(it != call.arguments.end() - 1)
            printf(", ");

        it++;
    }
    printf(")");
}

void print(const Condition& condition, const int indent)
{
    print_indent(indent);
    printf("if ");
    print(condition.condition, indent);
    printf(" then\n");

    print_statements(condition.statements, indent + 1);

    print_indent(indent);
    printf("end");
}

void print(const ForLoop& loop, const int indent)
{
    print_indent(indent);
    printf(
        "for %ld , %ld , %ld do\n",
        (int)loop.begin.value,
        (int)loop.end.value,
        (int)loop.increment.value);

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
    printf("while ");
    print(loop.condition, indent);
    printf(" do\n");

    print_statements(loop.statements, indent + 1);

    print_indent(indent);
    printf("end");
}
