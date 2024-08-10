#ifndef LUA4DEC_AST_H
#define LUA4DEC_AST_H

#include "lua/lua.hpp"

#include <sstream>
#include <variant>
#include <vector>

using StringBuffer = std::stringstream;

struct Closure;
struct Dotted;
struct Identifier;
struct Indexed;
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
struct LocalDefinition;
struct Return;
struct TailCall;
struct WhileLoop;

using Expression =
    std::variant<Call, Closure, Dotted, Identifier, Indexed, AstInt, AstList, AstMap, AstNumber, AstOperation, AstString, AstTable>;
using Statement =
    std::variant<Assignment, Call, Condition, ForLoop, ForInLoop, LocalDefinition, Return, TailCall, WhileLoop>;
using AstElement = std::variant<Statement, Expression>;

struct Context
{
    unsigned jump_offset  = 0;
    unsigned jmp_offset   = 0;
    bool     is_jmp       = false;
    bool     is_condition = false;
    bool     is_jmp_block = false;
    bool     is_or_block  = false;
};

struct Ast
{
    Ast*              child;
    Ast*              parent;
    Context           context;
    Vector<Statement> statements;
};

// Expressions

struct Closure
{
    Vector<Statement>  statements;
    Vector<Identifier> arguments;

    Closure(const Vector<Statement>& s, const Vector<Identifier> a)
        : statements(s)
        , arguments(a)
    {
    }
};

struct Dotted
{
    Vector<Expression> ex;

    Dotted(const Vector<Expression>& e)
        : ex(e)
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

struct Indexed
{
    Vector<Expression> ex;

    Indexed(const Vector<Expression>& e)
        : ex(e)
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
    Vector<Expression> elements;

    AstList(const Vector<Expression>& e)
        : elements(e)
    {
    }
};

struct AstMap
{
    Vector<std::pair<Expression, Expression>> pairs;

    AstMap(const Vector<std::pair<Expression, Expression>>& p)
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
    std::string        op;
    Vector<Expression> ex;

    AstOperation(const std::string& o, const Vector<Expression>& e)
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
    Identifier                                name;
    unsigned                                  size;
    Vector<std::pair<Expression, Expression>> pairs;

    AstTable(
        const unsigned                                   s,
        const Identifier&                                n,
        const Vector<std::pair<Expression, Expression>>& p)
        : size(s)
        , name(n)
        , pairs(p)
    {
    }
};

struct Call
{
    Vector<Expression> caller;
    Vector<Expression> arguments;
    unsigned           return_values;

    Call(const Vector<Expression>& c, const Vector<Expression>& a, const unsigned r = 0)
        : caller(c)
        , arguments(a)
        , return_values(r)
    {
    }
};

// Statements

struct Assignment
{
    Vector<Identifier> left;
    Vector<Expression> right;
    unsigned           num_variables;
    unsigned           num_values;

    Assignment(
        const Vector<Identifier>& i,
        const Vector<Expression>& e,
        const unsigned            vars = 1,
        const unsigned            vals = 1)
        : left(i)
        , right(e)
        , num_variables(vars)
        , num_values(vals)
    {
    }
};

struct ConditionBlock
{
    AstOperation      comparison;
    Vector<Statement> statements;

    ConditionBlock(const AstOperation& o, const Vector<Statement>& s)
        : comparison(o)
        , statements(s)
    {
    }
};

struct Condition
{
    Vector<ConditionBlock> blocks;

    Condition(const Vector<ConditionBlock>& c)
        : blocks(c)
    {
    }
};

struct ForLoop
{
    String            counter;
    Expression        begin;
    Expression        end;
    Expression        increment;
    Vector<Statement> statements;

    ForLoop(
        const String&            c,
        const Expression         b,
        const Expression         e,
        const Expression         i,
        const Vector<Statement>& s)
        : counter(c)
        , begin(b)
        , end(e)
        , increment(i)
        , statements(s)
    {
    }
};

struct ForInLoop
{
    String            key;
    String            value;
    Expression        table;
    Vector<Statement> statements;

    ForInLoop(const String& k, const String& v, const Expression& t, const Vector<Statement>& s)
        : key(k)
        , value(v)
        , table(t)
        , statements(s)
    {
    }
};

struct LocalDefinition
{
    Vector<Identifier> left;
    Vector<Expression> right;

    LocalDefinition(const Vector<Identifier>& l, const Vector<Expression>& r)
        : left(l)
        , right(r)
    {
    }
};

struct Return
{
    Vector<Expression> ex;

    Return(const Vector<Expression>& e)
        : ex(e)
    {
    }
};

struct TailCall
{
    Vector<Expression> caller;
    Vector<Expression> arguments;

    TailCall(const Vector<Expression>& c, const Vector<Expression>& a)
        : caller(c)
        , arguments(a)
    {
    }
};

struct WhileLoop
{
    AstOperation      condition;
    Vector<Statement> statements;

    WhileLoop(const AstOperation& o, const Vector<Statement>& s)
        : condition(o)
        , statements(s)
    {
    }
};

/*
 * Stuff to print the AST
 */

void print_ast(const Ast*, FILE* stream = stdout);
void print_ast(const Ast*, StringBuffer&);

void print_indent(const int, StringBuffer&);

void print_statements(const Vector<Statement>&, StringBuffer&, const int indent = 0);
void print_statement(const Statement&, StringBuffer&, const int indent = 0);
void print_expression(const Expression&, StringBuffer&, const int indent = 0);

void print(const Closure&, FILE* stream = stdout, const int indent = 0);
void print(const Closure&, StringBuffer&, const int indent = 0);
void print(const Dotted&, StringBuffer&, const int indent = 0);
void print(const Identifier&, StringBuffer&, const int indent = 0);
void print(const Indexed&, StringBuffer&, const int indent = 0);
void print(const AstInt&, StringBuffer&, const int indent = 0);
void print(const AstList&, StringBuffer&, const int indent = 0);
void print(const AstMap&, StringBuffer&, const int indent = 0);
void print(const AstNumber&, StringBuffer&, const int indent = 0);
void print(const AstOperation&, StringBuffer&, const int indent = 0);
void print(const AstString&, StringBuffer&, const int indent = 0);
void print(const AstTable&, StringBuffer&, const int indent = 0);

void print(const Assignment&, StringBuffer&, const int indent = 0);
void print(const Call&, StringBuffer&, const int indent = 0);
void print(const Condition&, StringBuffer&, const int indent = 0);
void print(const ForLoop&, StringBuffer&, const int indent = 0);
void print(const ForInLoop&, StringBuffer&, const int indent = 0);
void print(const LocalDefinition&, StringBuffer&, const int indent = 0);
void print(const Return&, StringBuffer&, const int indent = 0);
void print(const TailCall&, StringBuffer&, const int indent = 0);
void print(const WhileLoop&, StringBuffer&, const int indent = 0);

#endif  // LUA4DEC_AST_H
