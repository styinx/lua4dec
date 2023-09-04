#include <string>
#include <variant>
#include <vector>

struct Assignment;
struct Call;
struct ForLoop;
struct ForInLoop;
struct WhileLoop;
struct Condition;

using Statement = std::variant<Assignment, Call, ForLoop, ForInLoop, WhileLoop, Condition>;

struct Ast
{
    std::vector<std::string> stack;
    std::vector<Statement>   statements;
    Ast*                     body;
    Ast*                     parent;
};

struct Assignment
{
    std::string left;
    std::string right;
};

struct Call
{
    std::string              name;
    std::vector<std::string> arguments;
};

struct ForLoop
{
    std::string            begin;
    std::string            end;
    std::string            increment;
    std::vector<Statement> statements;
};

struct ForInLoop
{
    std::string            left = "temp";
    std::string            right;
    std::vector<Statement> statements;
};

struct WhileLoop
{
    std::string            condition;
    std::vector<Statement> statements;
};

struct Condition
{
    std::string            condition;
    std::vector<Statement> statements;
};


/*
 * Stuff to print the AST
 */

void print_ast(const Ast &);

void print_indent(const int indent);

void print_statements(const std::vector<Statement>&, const int indent = 0);

void print(const Assignment &, const int indent = 0);
void print(const Call &, const int indent = 0);
void print(const ForLoop &, const int indent = 0);
void print(const ForInLoop &, const int indent = 0);
void print(const WhileLoop &, const int indent = 0);
void print(const Condition &, const int indent = 0);
