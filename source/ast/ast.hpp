#include "lua/lua.hpp"
#include <variant>
#include <vector>

struct Closure;
struct Identifier;
struct AstInt;
struct AstList;
struct AstMap;
struct AstNumber;
struct AstOperation;
struct AstString;

struct Assignment;
struct Call;
struct Condition;
struct ForLoop;
struct ForInLoop;
struct LocalAssignment;
struct TailCall;
struct WhileLoop;

using Expression =
    std::variant<Closure, Identifier, AstInt, AstList, AstMap, AstNumber, AstOperation, AstString>;
using Statement =
    std::variant<Assignment, Call, Condition, ForLoop, ForInLoop, LocalAssignment, TailCall, WhileLoop>;
using AstElement = std::variant<Statement, Expression>;

struct Ast
{
    Collection<AstElement> stack;
    Collection<Statement>  statements;
    Ast*                   body;
    Ast*                   parent;
};

// Expressions

struct Closure
{
    Collection<Statement>       statements;
    Collection<LocalAssignment> arguments;

    Closure(const Collection<Statement>& s, const Collection<LocalAssignment> a)
        : statements(s)
        , arguments(a)
    {
    }
};

struct Identifier
{
    String name;

    Identifier(const String& n)
        : name(n)
    {
    }
};

struct AstInt
{
    int value;

    AstInt(const int& v)
        : value(v)
    {
    }
};

struct AstList
{
    Collection<Expression> elements;

    AstList(const Collection<Expression>& e)
        : elements(e)
    {
    }
};

struct AstMap
{
    Collection<std::pair<Expression, Expression>> pairs;

    AstMap(const Collection<std::pair<Expression, Expression>>& p)
        : pairs(p)
    {
    }
};

struct AstNumber
{
    Number value;

    AstNumber(const Number& v)
        : value(v)
    {
    }
};

struct AstOperation
{
    std::string            op;
    Collection<Expression> ex;

    AstOperation(const std::string& o, const Collection<Expression>& e)
        : op(o)
        , ex(e)
    {
    }
};

struct AstString
{
    String value;

    AstString(const String& v)
        : value(v)
    {
    }
};

// Statements

struct Assignment
{
    Identifier left;
    Expression right;

    Assignment(const Identifier& i, const Expression& e)
        : left(i)
        , right(e)
    {
    }
};

struct Call
{
    Identifier             name;
    Collection<Expression> arguments;

    Call(const Identifier& i, const Collection<Expression>& a)
        : name(i)
        , arguments(a)
    {
    }
};

struct Condition
{
    AstOperation          condition;
    Collection<Statement> statements;

    Condition(const AstOperation& o, const Collection<Statement>& s)
        : condition(o)
        , statements(s)
    {
    }
};

struct ForLoop
{
    AstInt                begin;
    AstInt                end;
    AstInt                increment;
    Collection<Statement> statements;

    ForLoop(const AstInt b, const AstInt e, const AstInt i, const Collection<Statement>& s)
        : begin(b)
        , end(e)
        , increment(i)
        , statements(s)
    {
    }
};

struct ForInLoop
{
    String                left = "temp";
    String                right;
    Collection<Statement> statements;
};

struct LocalAssignment
{
    Identifier left;
    Expression right;

    LocalAssignment(const Identifier& i, const Expression& e)
        : left(i)
        , right(e)
    {
    }
};

struct TailCall
{
    Identifier             name;
    Collection<Expression> arguments;

    TailCall(const Identifier& i, const Collection<Expression>& a)
        : name(i)
        , arguments(a)
    {
    }
};

struct WhileLoop
{
    AstOperation          condition;
    Collection<Statement> statements;

    WhileLoop(const AstOperation& o, const Collection<Statement>& s)
        : condition(o)
        , statements(s)
    {
    }
};

/*
 * Stuff to print the AST
 */

void print_stack(const Ast& ast);

void print_ast(const Ast&);

void print_indent(const int indent);

void print_statements(const Collection<Statement>&, const int indent = 0);

void print(const Closure&, const int indent = 0);
void print(const Identifier&, const int indent = 0);
void print(const AstInt&, const int indent = 0);
void print(const AstList&, const int indent = 0);
void print(const AstMap&, const int indent = 0);
void print(const AstNumber&, const int indent = 0);
void print(const AstOperation&, const int indent = 0);
void print(const AstString&, const int indent = 0);

void print(const Assignment&, const int indent = 0);
void print(const Call&, const int indent = 0);
void print(const Condition&, const int indent = 0);
void print(const ForLoop&, const int indent = 0);
void print(const ForInLoop&, const int indent = 0);
void print(const LocalAssignment&, const int indent = 0);
void print(const TailCall&, const int indent = 0);
void print(const WhileLoop&, const int indent = 0);
