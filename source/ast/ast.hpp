#ifndef LUA4DEC_AST_H
#define LUA4DEC_AST_H

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
struct AstTable;

struct Assignment;
struct Call;
struct Condition;
struct ForLoop;
struct ForInLoop;
struct LocalAssignment;
struct TailCall;
struct WhileLoop;

using Expression =
    std::variant<Call, Closure, Identifier, AstInt, AstList, AstMap, AstNumber, AstOperation, AstString, AstTable>;
using Statement =
    std::variant<Assignment, Call, Condition, ForLoop, ForInLoop, LocalAssignment, TailCall, WhileLoop>;
using AstElement = std::variant<Statement, Expression>;

struct Context
{
    unsigned jump_offset  = 0;
    unsigned PC           = 0;
    bool     is_condition = false;
};

struct Ast
{
    Ast*                   child;
    Ast*                   parent;
    Context                context;
    Collection<AstElement> stack;
    Collection<Statement>  statements;
};

// Expressions

struct Closure
{
    Collection<Statement>       statements;
    Collection<Identifier>      arguments;
    Collection<LocalAssignment> locals;

    Closure(
        const Collection<Statement>&       s,
        const Collection<Identifier>       a,
        const Collection<LocalAssignment>& l)
        : statements(s)
        , arguments(a)
        , locals(l)
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

    bool empty() const
    {
        return op.empty() && ex.empty();
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

struct AstTable
{
    Identifier                                    name;
    unsigned                                      size;
    Collection<std::pair<Expression, Expression>> pairs;

    AstTable(
        const unsigned                                       s,
        const Identifier&                                    n,
        const Collection<std::pair<Expression, Expression>>& p)
        : size(s)
        , name(n)
        , pairs(p)
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

struct ConditionBlock
{
    AstOperation          comparison;
    Collection<Statement> statements;

    ConditionBlock(const AstOperation& o, const Collection<Statement>& s)
        : comparison(o)
        , statements(s)
    {
    }
};

struct Condition
{
    Collection<ConditionBlock> blocks;

    Condition(const Collection<ConditionBlock>& c)
        : blocks(c)
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

void print_stack(const Ast&, FILE* stream = stdout);

void print_ast(const Ast&, FILE* stream = stdout);

void print_indent(const int, FILE* stream = stdout);

void print_statements(const Collection<Statement>&, const int indent = 0, FILE* stream = stdout);

void print(const Closure&, const int indent = 0, FILE* stream = stdout);
void print(const Identifier&, const int indent = 0, FILE* stream = stdout);
void print(const AstInt&, const int indent = 0, FILE* stream = stdout);
void print(const AstList&, const int indent = 0, FILE* stream = stdout);
void print(const AstMap&, const int indent = 0, FILE* stream = stdout);
void print(const AstNumber&, const int indent = 0, FILE* stream = stdout);
void print(const AstOperation&, const int indent = 0, FILE* stream = stdout);
void print(const AstString&, const int indent = 0, FILE* stream = stdout);
void print(const AstTable&, const int indent = 0, FILE* stream = stdout);

void print(const Assignment&, const int indent = 0, FILE* stream = stdout);
void print(const Call&, const int indent = 0, FILE* stream = stdout);
void print(const Condition&, const int indent = 0, FILE* stream = stdout);
void print(const ForLoop&, const int indent = 0, FILE* stream = stdout);
void print(const ForInLoop&, const int indent = 0, FILE* stream = stdout);
void print(const LocalAssignment&, const int indent = 0, FILE* stream = stdout);
void print(const TailCall&, const int indent = 0, FILE* stream = stdout);
void print(const WhileLoop&, const int indent = 0, FILE* stream = stdout);

#endif  // LUA4DEC_AST_H
