#include "ast/ast.hpp"

const char INDENT_SIZE = 2;

void print_ast(const Ast* ast, FILE* stream)
{
    StringBuffer buffer;
    print_ast(ast, buffer);
    fprintf(stream, buffer.str().c_str());
}

void print_ast(const Ast* ast, StringBuffer& buffer)
{
    print_statements(ast->statements, buffer, 0);
}

void print_indent(StringBuffer& buffer, const int indent)
{
    buffer << std::string(indent * INDENT_SIZE, ' ');
}

void print_statements(const std::vector<Statement>& statements, StringBuffer& buffer, const int indent)
{
    for(const auto& statement : statements)
    {
        print_statement(statement, buffer, indent);
        buffer << "\n";
    }
}

void print_statement(const Statement& statement, StringBuffer& buffer, const int indent)
{
    std::visit([&buffer, indent](auto&& s) { print(s, buffer, indent); }, statement);
}

void print_expression(const Expression& expression, StringBuffer& buffer, const int indent)
{
    std::visit([&buffer, indent](auto&& e) { print(e, buffer, indent); }, expression);
}

// Expressions

void print(const Closure& closure, StringBuffer& buffer, const int indent)
{
    buffer << "function(";

    for(const auto& arg : closure.arguments)
    {
        print(arg, buffer, indent + 1);

        if(&arg != &closure.arguments.back())
            buffer << ", ";
    }

    buffer << ")\n";

    print_statements(closure.statements, buffer, indent + 1);

    print_indent(buffer, indent);
    buffer << "end";
}

void print(const Dotted& dotted, StringBuffer& buffer, const int indent)
{
    print_expression(dotted.ex[0], buffer, indent);
    buffer << ".";
    print_expression(dotted.ex[1], buffer, 0);
}

void print(const Identifier& identifier, StringBuffer& buffer, const int indent)
{
    buffer << identifier.name;
}

void print(const Indexed& indexed, StringBuffer& buffer, const int indent)
{
    print_expression(indexed.ex[0], buffer, indent);
    buffer << "[";
    print_expression(indexed.ex[1], buffer, 0);
    buffer << "]";
}

void print(const AstInt& number, StringBuffer& buffer, const int indent)
{
    buffer << number.value;
}

void print(const AstList& list, StringBuffer& buffer, const int indent)
{
    buffer << "{";
    for(const auto& el : list.elements)
    {
        print_expression(el, buffer, indent);

        if(&el != &list.elements.back())
            buffer << ", ";
    }
    buffer << "}";
}

void print(const AstMap& map, StringBuffer& buffer, const int indent)
{
    buffer << "{";
    for(const auto& p : map.pairs)
    {
        print_indent(buffer, indent);
        buffer << std::get<AstString>(p.first).value;
        buffer << " = ";
        print_expression(p.second, buffer, indent);

        if(&p != &map.pairs.back())
            buffer << ", ";
    }
    buffer << "}";
}

void print(const AstNumber& number, StringBuffer& buffer, const int indent)
{
    buffer << number.value;
}

void print(const AstOperation& operation, StringBuffer& buffer, const int indent)
{
    if(operation.ex.size() == 1)
        buffer << operation.op;

    auto it = operation.ex.begin();
    while(it != operation.ex.end())
    {
        if(std::holds_alternative<AstOperation>(*it))
        {
            buffer << "(";
            print_expression(*it, buffer, indent);
            buffer << ")";
        }
        else
            print_expression(*it, buffer, indent);

        it++;

        if(it != operation.ex.end())
            buffer << " " << operation.op << " ";
    }
}

void print(const AstString& string, StringBuffer& buffer, const int indent)
{
    buffer << "\"" << string.value << "\"";
}

void print(const AstTable& table, StringBuffer& buffer, const int indent)
{
    buffer << table.name.name.c_str() << " {\n";
    for(const auto& p : table.pairs)
    {
        print_indent(buffer, indent + 1);
        if(std::holds_alternative<AstString>(p.first))
            buffer << std::get<AstString>(p.first).value;
        else if(std::holds_alternative<Identifier>(p.first))
            buffer << std::get<Identifier>(p.first).name;

        buffer << " = ";
        print_expression(p.second, buffer, indent + 1);

        if(&p != &table.pairs.back())
            buffer << ",\n";
    }
    buffer << "\n";
    print_indent(buffer, indent);
    buffer << "}";
}

// Statements

void print(const Assignment& assignment, StringBuffer& buffer, const int indent)
{
    print_indent(buffer, indent);

    auto lit = assignment.left.begin();
    while(lit != assignment.left.end())
    {
        buffer << lit->name;

        if(lit != assignment.left.end() - 1)
            buffer << ", ";

        lit++;
    }

    buffer << " = ";

    auto rit = assignment.right.begin();
    while(rit != assignment.right.end())
    {
        print_expression(*rit, buffer, 0);

        if(rit != assignment.right.end() - 1)
            buffer << ", ";

        rit++;
    }
}

void print(const Call& call, StringBuffer& buffer, const int indent)
{
    if(!(call.return_values > 0))
        print_indent(buffer, indent);

    print_expression(call.caller[0], buffer, 0);

    buffer << "(";
    auto it = call.arguments.begin();
    while(it != call.arguments.end())
    {
        print_expression(*it, buffer, 0);

        if(it != call.arguments.end() - 1)
            buffer << ", ";

        it++;
    }
    buffer << ")";
}

void print(const Condition& condition, StringBuffer& buffer, const int indent)
{
    for(auto it = condition.blocks.begin(); it != condition.blocks.end(); ++it)
    {
        print_indent(buffer, indent);

        if(it == condition.blocks.begin())
        {
            buffer << "if ";
            print(it->comparison, buffer, indent);
            buffer << " then\n";
            print_statements(it->statements, buffer, indent + 1);
        }
        else if(!it->comparison.empty())
        {
            buffer << "elseif ";
            print(it->comparison, buffer, indent);
            buffer << " then\n";
            print_statements(it->statements, buffer, indent + 1);
        }
        else
        {
            buffer << "else\n";
            print_statements(it->statements, buffer, indent + 1);
        }
    }

    print_indent(buffer, indent);
    buffer << "end";
}

void print(const ForLoop& loop, StringBuffer& buffer, const int indent)
{
    print_indent(buffer, indent);
    buffer << "for " << loop.counter << " = ";

    print_expression(loop.begin, buffer, 0);
    buffer << " , ";
    print_expression(loop.end, buffer, 0);
    buffer << " , ";
    print_expression(loop.increment, buffer, 0);

    buffer << " do\n";

    print_statements(loop.statements, buffer, indent + 1);

    print_indent(buffer, indent);
    buffer << "end";
}

void print(const ForInLoop& loop, StringBuffer& buffer, const int indent)
{
    print_indent(buffer, indent);
    buffer << "for " << loop.key << " , " << loop.value << " in ";

    print_expression(loop.table, buffer, 0);

    buffer << " do\n";

    print_statements(loop.statements, buffer, indent + 1);

    print_indent(buffer, indent);
    buffer << "end";
}

void print(const LocalDefinition& definition, StringBuffer& buffer, const int indent)
{
    print_indent(buffer, indent);
    buffer << "local ";

    auto key = definition.left.begin();
    while(key != definition.left.end())
    {
        buffer << key->name;
        if(key != definition.left.end() - 1)
            buffer << ", ";
        key++;
    }

    buffer << " = ";

    auto val = definition.right.begin();
    while(val != definition.right.end())
    {
        print_expression(*val, buffer, indent);
        if(val != definition.right.end() - 1)
            buffer << ", ";
        val++;
    }
}

void print(const Return& ret, StringBuffer& buffer, const int indent)
{
    print_indent(buffer, indent);
    buffer << "return ";

    auto it = ret.ex.begin();
    while(it != ret.ex.end())
    {
        print_expression(*it, buffer, indent);

        if(it != ret.ex.end() - 1)
            buffer << ", ";

        it++;
    }
}

void print(const TailCall& call, StringBuffer& buffer, const int indent)
{
    print_indent(buffer, indent);

    buffer << "return ";

    print_expression(call.caller[0], buffer, indent);

    buffer << "(";
    auto it = call.arguments.begin();
    while(it != call.arguments.end())
    {
        print_expression(*it, buffer, indent);

        if(it != call.arguments.end() - 1)
            buffer << ", ";

        it++;
    }
    buffer << ")";
}

void print(const WhileLoop& loop, StringBuffer& buffer, const int indent)
{
    print_indent(buffer, indent);
    buffer << "while ";
    print(loop.condition, buffer, indent);
    buffer << " do\n";

    print_statements(loop.statements, buffer, indent + 1);

    print_indent(buffer, indent);
    buffer << "end";
}
