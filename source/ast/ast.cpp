#include "ast/ast.hpp"

void print_stack(const Ast& ast, FILE* stream)
{
    fprintf(stream, " === STACK ===\n");
    for(const auto& e : ast.stack)
    {
        // TODO
    }
    fprintf(stream, " =============\n");
}

void print_ast(const Ast& ast, FILE* stream)
{
    print_statements(ast.statements, 0, stream);
}

void print_indent(const int indent, FILE* stream)
{
    fprintf(stream, "%*s", indent * 2, "");
}

void print_statements(const std::vector<Statement>& statements, const int indent, FILE* stream)
{
    for(const auto& statement : statements)
    {
        print_indent(indent, stream);
        std::visit([indent, stream](auto&& s) { print(s, indent, stream); }, statement);
        fprintf(stream, "\n");
    }
}

// Expressions

void print(const Closure& closure, const int indent, FILE* stream)
{
    fprintf(stream, "function(");

    for(const auto& arg : closure.arguments)
    {
        print(arg, indent + 1, stream);

        if(&arg != &closure.arguments.back())
            fprintf(stream, ", ");
    }

    fprintf(stream, ")\n");

    for(const auto& local : closure.locals)
    {
        print(local, indent + 1, stream);
        fprintf(stream, "\n");
    }

    print_statements(closure.statements, indent + 1, stream);

    print_indent(indent, stream);
    fprintf(stream, "end");
}

void print(const Identifier& identifier, const int /*indent*/, FILE* stream)
{
    fprintf(stream, "%s", identifier.name.c_str());
}

void print(const AstInt& number, const int /*indent*/, FILE* stream)
{
    fprintf(stream, "%ld", number.value);
}

void print(const AstList& list, const int indent, FILE* stream)
{
    fprintf(stream, "{");
    for(const auto& el : list.elements)
    {
        std::visit([indent, stream](auto&& e) { print(e, indent, stream); }, el);

        if(&el != &list.elements.back())
            fprintf(stream, ", ");
    }
    fprintf(stream, "}");
}

void print(const AstMap& map, const int indent, FILE* stream)
{
    fprintf(stream, "{");
    for(const auto& p : map.pairs)
    {
        print_indent(indent, stream);
        fprintf(stream, "%s", std::get<AstString>(p.first).value.c_str());
        fprintf(stream, " = ");
        std::visit([indent, stream](auto&& e) { print(e, indent, stream); }, p.second);

        if(&p != &map.pairs.back())
            fprintf(stream, ", ");
    }
    fprintf(stream, "}");
}

void print(const AstNumber& number, const int /*indent*/, FILE* stream)
{
    fprintf(stream, "%lf", number.value);
}

void print(const AstOperation& operation, const int indent, FILE* stream)
{
    if(operation.ex.size() == 1)
        fprintf(stream, "%s ", operation.op.c_str());

    auto it = operation.ex.rbegin();
    while(it != operation.ex.rend())
    {
        std::visit([indent, stream](auto&& e) { print(e, indent, stream); }, *it);

        it++;

        if(it != operation.ex.rend())
            fprintf(stream, " %s ", operation.op.c_str());
    }
}

void print(const AstString& string, const int /*indent*/, FILE* stream)
{
    fprintf(stream, "\"%s\"", string.value.c_str());
}

void print(const AstTable& table, const int indent, FILE* stream)
{
    fprintf(stream, "%s {\n", table.name.name.c_str());
    for(const auto& p : table.pairs)
    {
        print_indent(indent + 1, stream);
        if(std::holds_alternative<AstString>(p.first))
            fprintf(stream, "%s", std::get<AstString>(p.first).value.c_str());
        else if(std::holds_alternative<Identifier>(p.first))
            fprintf(stream, "%s", std::get<Identifier>(p.first).name.c_str());

        fprintf(stream, " = ");
        std::visit([indent, stream](auto&& e) { print(e, indent + 1, stream); }, p.second);

        if(&p != &table.pairs.back())
            fprintf(stream, ",\n");
    }
    fprintf(stream, "\n");
    print_indent(indent, stream);
    fprintf(stream, "}");
}

// Statements

void print(const Assignment& assignment, const int indent, FILE* stream)
{
    print_indent(indent, stream);
    fprintf(stream, "%s = ", assignment.left.name.c_str());
    std::visit([indent, stream](auto&& e) { print(e, indent, stream); }, assignment.right);
}

void print(const Call& call, const int indent, FILE* stream)
{
    print_indent(indent, stream);
    fprintf(stream, "%s(", call.name.name.c_str());

    auto it = call.arguments.begin();
    while(it != call.arguments.end())
    {
        std::visit([indent, stream](auto&& e) { print(e, indent, stream); }, *it);

        if(it != call.arguments.end() - 1)
            fprintf(stream, ", ");

        it++;
    }
    fprintf(stream, ")");
}

void print(const Condition& condition, const int indent, FILE* stream)
{

    for(auto it = condition.blocks.begin(); it != condition.blocks.end(); ++it)
    {
        print_indent(indent, stream);

        if(it == condition.blocks.begin())
        {
            fprintf(stream, "if ");
            print(it->comparison, indent, stream);
            fprintf(stream, " then\n");
            print_statements(it->statements, indent + 1, stream);
        }
        else if(!it->comparison.empty())
        {
            fprintf(stream, "elseif ");
            print(it->comparison, indent, stream);
            fprintf(stream, " then\n");
            print_statements(it->statements, indent + 1, stream);
        }
        else
        {
            fprintf(stream, "else\n");
            print_statements(it->statements, indent + 1, stream);
        }
    }

    print_indent(indent, stream);
    fprintf(stream, "end");
}

void print(const ForLoop& loop, const int indent, FILE* stream)
{
    print_indent(indent, stream);
    fprintf(
        stream,
        "for %ld , %ld , %ld do\n",
        (int)loop.begin.value,
        (int)loop.end.value,
        (int)loop.increment.value);

    print_statements(loop.statements, indent + 1, stream);

    print_indent(indent, stream);
    fprintf(stream, "end");
}

void print(const ForInLoop& loop, const int indent, FILE* stream)
{
    print_indent(indent, stream);
    fprintf(stream, "for %s in %s do\n", loop.left.c_str(), loop.right.c_str());

    print_statements(loop.statements, indent + 1, stream);

    print_indent(indent, stream);
    fprintf(stream, "end");
}

void print(const LocalAssignment& assignment, const int indent, FILE* stream)
{
    print_indent(indent, stream);
    fprintf(stream, "local %s = ", assignment.left.name.c_str());
    std::visit([indent, stream](auto&& e) { print(e, indent, stream); }, assignment.right);
}

void print(const TailCall& call, const int indent, FILE* stream)
{
    print_indent(indent, stream);
    fprintf(stream, "return %s(", call.name.name.c_str());

    auto it = call.arguments.begin();
    while(it != call.arguments.end())
    {
        std::visit([indent, stream](auto&& e) { print(e, indent, stream); }, *it);

        if(it != call.arguments.end() - 1)
            fprintf(stream, ", ");

        it++;
    }
    fprintf(stream, ")");
}

void print(const WhileLoop& loop, const int indent, FILE* stream)
{
    print_indent(indent, stream);
    fprintf(stream, "while ");
    print(loop.condition, indent, stream);
    fprintf(stream, " do\n");

    print_statements(loop.statements, indent + 1, stream);

    print_indent(indent, stream);
    fprintf(stream, "end");
}
