#include "ast/ast.hpp"

void print_ast(const Ast* ast, FILE* stream)
{
    print_statements(ast->statements, stream, 0);
}

void print_ast(const Ast* ast, StringBuffer& buffer)
{
    print_statements(ast->statements, buffer, 0);
}

void print_indent(FILE* stream, const int indent)
{
    fprintf(stream, "%*s", indent * 2, "");
}

void print_indent(StringBuffer& buffer, const int indent)
{
    // sprintf(buffer, "%*s", indent * 2, "");
}

void print_statements(const std::vector<Statement>& statements, FILE* stream, const int indent)
{
    for(const auto& statement : statements)
    {
        print_indent(stream, indent);
        std::visit([&stream, indent](auto&& s) { print(s, stream, indent); }, statement);
        fprintf(stream, "\n");
    }
}

void print_statements(const std::vector<Statement>& statements, StringBuffer& buffer, const int indent)
{
    for(const auto& statement : statements)
    {
        print_indent(buffer, indent);
        std::visit([&buffer, indent](auto&& s) { print(s, buffer, indent); }, statement);
        // sprintf(buffer, "\n");
    }
}

// Expressions

void print(const Closure& closure, FILE* stream, const int indent)
{
    fprintf(stream, "function(");

    for(const auto& arg : closure.arguments)
    {
        print(arg, stream, indent + 1);

        if(&arg != &closure.arguments.back())
            fprintf(stream, ", ");
    }

    fprintf(stream, ")\n");

    for(const auto& local : closure.locals)
    {
        print(local, stream, indent + 1);
        fprintf(stream, "\n");
    }

    print_statements(closure.statements, stream, indent + 1);

    print_indent(stream, indent);
    fprintf(stream, "end");
}

void print(const Closure& closure, StringBuffer& buffer, const int indent)
{
}

void print(const Identifier& identifier, FILE* stream, const int /*indent*/)
{
    fprintf(stream, "%s", identifier.name.c_str());
}

void print(const Identifier& identifier, StringBuffer& buffer, const int indent)
{
    buffer << identifier.name;
}

void print(const AstInt& number, FILE* stream, const int /*indent*/)
{
    fprintf(stream, "%ld", number.value);
}

void print(const AstInt& number, StringBuffer& buffer, const int indent)
{
}

void print(const AstList& list, FILE* stream, const int indent)
{
    fprintf(stream, "{");
    for(const auto& el : list.elements)
    {
        std::visit([stream, indent](auto&& e) { print(e, stream, indent); }, el);

        if(&el != &list.elements.back())
            fprintf(stream, ", ");
    }
    fprintf(stream, "}");
}

void print(const AstList& list, StringBuffer& buffer, const int indent)
{
}

void print(const AstMap& map, FILE* stream, const int indent)
{
    fprintf(stream, "{");
    for(const auto& p : map.pairs)
    {
        print_indent(stream, indent);
        fprintf(stream, "%s", std::get<AstString>(p.first).value.c_str());
        fprintf(stream, " = ");
        std::visit([stream, indent](auto&& e) { print(e, stream, indent); }, p.second);

        if(&p != &map.pairs.back())
            fprintf(stream, ", ");
    }
    fprintf(stream, "}");
}

void print(const AstMap& map, StringBuffer& buffer, const int indent)
{
}

void print(const AstNumber& number, FILE* stream, const int /*indent*/)
{
    fprintf(stream, "%lf", number.value);
}

void print(const AstNumber& number, StringBuffer& buffer, const int indent)
{
}

void print(const AstOperation& operation, FILE* stream, const int indent)
{
    if(operation.ex.size() == 1)
        fprintf(stream, "%s ", operation.op.c_str());

    auto it = operation.ex.rbegin();
    while(it != operation.ex.rend())
    {
        std::visit([stream, indent](auto&& e) { print(e, stream, indent); }, *it);

        it++;

        if(it != operation.ex.rend())
            fprintf(stream, " %s ", operation.op.c_str());
    }
}

void print(const AstOperation& operation, StringBuffer& buffer, const int indent)
{
}

void print(const AstString& string, FILE* stream, const int /*indent*/)
{
    fprintf(stream, "\"%s\"", string.value.c_str());
}

void print(const AstString& operation, StringBuffer& buffer, const int indent)
{
}

void print(const AstTable& table, FILE* stream, const int indent)
{
    fprintf(stream, "%s {\n", table.name.name.c_str());
    for(const auto& p : table.pairs)
    {
        print_indent(stream, indent + 1);
        if(std::holds_alternative<AstString>(p.first))
            fprintf(stream, "%s", std::get<AstString>(p.first).value.c_str());
        else if(std::holds_alternative<Identifier>(p.first))
            fprintf(stream, "%s", std::get<Identifier>(p.first).name.c_str());

        fprintf(stream, " = ");
        std::visit([stream, indent](auto&& e) { print(e, stream, indent + 1); }, p.second);

        if(&p != &table.pairs.back())
            fprintf(stream, ",\n");
    }
    fprintf(stream, "\n");
    print_indent(stream, indent);
    fprintf(stream, "}");
}

void print(const AstTable& table, StringBuffer& buffer, const int indent)
{
}

// Statements

void print(const Assignment& assignment, FILE* stream, const int indent)
{
    print_indent(stream, indent);
    fprintf(stream, "%s = ", assignment.left.name.c_str());
    std::visit([stream, indent](auto&& e) { print(e, stream, indent); }, assignment.right);
}

void print(const Assignment& assignment, StringBuffer& buffer, const int indent)
{
    buffer << assignment.left.name;
}

void print(const Call& call, FILE* stream, const int indent)
{
    print_indent(stream, indent);
    fprintf(stream, "%s(", call.name.name.c_str());

    auto it = call.arguments.begin();
    while(it != call.arguments.end())
    {
        std::visit([stream, indent](auto&& e) { print(e, stream, indent); }, *it);

        if(it != call.arguments.end() - 1)
            fprintf(stream, ", ");

        it++;
    }
    fprintf(stream, ")");
}

void print(const Call& call, StringBuffer& buffer, const int indent)
{
}

void print(const Condition& condition, FILE* stream, const int indent)
{

    for(auto it = condition.blocks.begin(); it != condition.blocks.end(); ++it)
    {
        print_indent(stream, indent);

        if(it == condition.blocks.begin())
        {
            fprintf(stream, "if ");
            print(it->comparison, stream, indent);
            fprintf(stream, " then\n");
            print_statements(it->statements, stream, indent + 1);
        }
        else if(!it->comparison.empty())
        {
            fprintf(stream, "elseif ");
            print(it->comparison, stream, indent);
            fprintf(stream, " then\n");
            print_statements(it->statements, stream, indent + 1);
        }
        else
        {
            fprintf(stream, "else\n");
            print_statements(it->statements, stream, indent + 1);
        }
    }

    print_indent(stream, indent);
    fprintf(stream, "end");
}

void print(const Condition& condition, StringBuffer& buffer, const int indent)
{
}

void print(const ForLoop& loop, FILE* stream, const int indent)
{
    print_indent(stream, indent);
    fprintf(stream, "for %s = ", loop.counter.c_str());

    std::visit([stream, indent](auto&& e) { print(e, stream, indent); }, loop.begin);
    fprintf(stream, " , ");
    std::visit([stream, indent](auto&& e) { print(e, stream, indent); }, loop.end);
    fprintf(stream, " , ");
    std::visit([stream, indent](auto&& e) { print(e, stream, indent); }, loop.increment);

    fprintf(stream, "do\n");

    print_statements(loop.statements, stream, indent + 1);

    print_indent(stream, indent);
    fprintf(stream, "end");
}

void print(const ForLoop& loop, StringBuffer& buffer, const int indent)
{
}

void print(const ForInLoop& loop, FILE* stream, const int indent)
{
    print_indent(stream, indent);
    fprintf(
        stream,
        "for %s , %s in %s do\n",
        loop.key.c_str(),
        loop.value.c_str(),
        loop.right.c_str());

    print_statements(loop.statements, stream, indent + 1);

    print_indent(stream, indent);
    fprintf(stream, "end");
}

void print(const ForInLoop& loop, StringBuffer& buffer, const int indent)
{
}

void print(const LocalAssignment& assignment, FILE* stream, const int indent)
{
    print_indent(stream, indent);
    fprintf(stream, "local %s = ", assignment.left.name.c_str());
    std::visit([stream, indent](auto&& e) { print(e, stream, indent); }, assignment.right);
}

void print(const LocalAssignment& assignment, StringBuffer& buffer, const int indent)
{
}

void print(const Return& ret, FILE* stream, const int indent)
{
    print_indent(stream, indent);
    fprintf(stream, "return ");

    auto it = ret.ex.begin();
    while(it != ret.ex.end())
    {
        std::visit([stream, indent](auto&& e) { print(e, stream, indent); }, *it);

        if(it != ret.ex.end() - 1)
            fprintf(stream, ", ");

        it++;
    }
}

void print(const Return& ret, StringBuffer& buffer, const int indent)
{
}

void print(const TailCall& call, FILE* stream, const int indent)
{
    print_indent(stream, indent);
    fprintf(stream, "return %s(", call.name.name.c_str());

    auto it = call.arguments.begin();
    while(it != call.arguments.end())
    {
        std::visit([stream, indent](auto&& e) { print(e, stream, indent); }, *it);

        if(it != call.arguments.end() - 1)
            fprintf(stream, ", ");

        it++;
    }
    fprintf(stream, ")");
}

void print(const TailCall& call, StringBuffer& buffer, const int indent)
{
}

void print(const WhileLoop& loop, FILE* stream, const int indent)
{
    print_indent(stream, indent);
    fprintf(stream, "while ");
    print(loop.condition, stream, indent);
    fprintf(stream, " do\n");

    print_statements(loop.statements, stream, indent + 1);

    print_indent(stream, indent);
    fprintf(stream, "end");
}

void print(const WhileLoop& loop, StringBuffer& buffer, const int indent)
{
}
