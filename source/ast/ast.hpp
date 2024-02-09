#ifndef LUA4DEC_AST_H
#define LUA4DEC_AST_H

#include "lua/lua.hpp"

#include <sstream>
#include <variant>
#include <vector>

using StringBuffer = std::stringstream;

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
struct Return;
struct TailCall;
struct WhileLoop;

using Expression =
    std::variant<Call, Closure, Identifier, AstInt, AstList, AstMap, AstNumber, AstOperation, AstString, AstTable>;
using Statement =
    std::variant<Assignment, Call, Condition, ForLoop, ForInLoop, LocalAssignment, Return, TailCall, WhileLoop>;
using AstElement = std::variant<Statement, Expression>;

struct Context
{
    unsigned jump_offset  = 0;
    bool     is_condition = false;
    bool     is_jmp_block = false;
    bool     is_or_block  = false;
};

struct Ast
{
    Ast*               child;
    Ast*               parent;
    Context            context;
    Vector<AstElement> stack;
    Vector<Statement>  statements;
};

// Expressions

struct Closure
{
    Vector<Statement>       statements;
    Vector<Identifier>      arguments;
    Vector<LocalAssignment> locals;

    Closure(const Vector<Statement>& s, const Vector<Identifier> a, const Vector<LocalAssignment>& l)
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
    Identifier         name;
    Vector<Expression> arguments;

    Call(const Identifier& i, const Vector<Expression>& a)
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
    String            right;
    Vector<Statement> statements;

    ForInLoop(const String& k, const String& v, const String& r, const Vector<Statement>& s)
        : key(k)
        , value(v)
        , right(r)
        , statements(s)
    {
    }
};

struct LocalAssignment
{
    Identifier left;
    Expression right;

    LocalAssignment(const Identifier& l, const Expression& r)
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
    Identifier         name;
    Vector<Expression> arguments;

    TailCall(const Identifier& i, const Vector<Expression>& a)
        : name(i)
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
void print_ast(const Ast*, StringBuffer& buffer);

void print_indent(const int, StringBuffer& buffer);

void print_statements(const Vector<Statement>&, StringBuffer& buffer, const int indent = 0);

void print(const Closure&, FILE* stream = stdout, const int indent = 0);
void print(const Closure&, StringBuffer& buffer, const int indent = 0);
void print(const Identifier&, StringBuffer& buffer, const int indent = 0);
void print(const AstInt&, StringBuffer& buffer, const int indent = 0);
void print(const AstList&, StringBuffer& buffer, const int indent = 0);
void print(const AstMap&, StringBuffer& buffer, const int indent = 0);
void print(const AstNumber&, StringBuffer& buffer, const int indent = 0);
void print(const AstOperation&, StringBuffer& buffer, const int indent = 0);
void print(const AstString&, StringBuffer& buffer, const int indent = 0);
void print(const AstTable&, StringBuffer& buffer, const int indent = 0);

void print(const Assignment&, StringBuffer& buffer, const int indent = 0);
void print(const Call&, StringBuffer& buffer, const int indent = 0);
void print(const Condition&, StringBuffer& buffer, const int indent = 0);
void print(const ForLoop&, StringBuffer& buffer, const int indent = 0);
void print(const ForInLoop&, StringBuffer& buffer, const int indent = 0);
void print(const LocalAssignment&, StringBuffer& buffer, const int indent = 0);
void print(const Return&, StringBuffer& buffer, const int indent = 0);
void print(const TailCall&, StringBuffer& buffer, const int indent = 0);
void print(const WhileLoop&, StringBuffer& buffer, const int indent = 0);

#endif  // LUA4DEC_AST_H
