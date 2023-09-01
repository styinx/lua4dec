#undef NDEBUG

#include <assert.h>
#include <limits>
#include <set>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

using byte         = unsigned char;
using ByteIterator = byte*;
using String       = std::string;
template<typename T>
using Collection = std::vector<T>;

#ifdef OS_32
using Instruction = unsigned;
using SizeT       = unsigned;
using Number      = float;
#else
using Instruction = unsigned;  // TODO: it depends ...
using SizeT       = size_t;
using Number      = double;
#endif

const unsigned MAX_INT = 2147483647 - 2;

enum class Operator : byte
{
    END = 0x00,
    RETURN,
    CALL,
    TAILCALL,
    PUSHNIL,
    POP,
    PUSHINT,
    PUSHSTRING,
    PUSHNUM = 0x08,
    PUSHNEGNUM,
    PUSHUPVALUE,
    GETLOCAL,
    GETGLOBAL,
    GETTABLE,
    GETDOTTED,
    GETINDEXED,
    PUSHSELF = 0x10,
    CREATETABLE,
    SETLOCAL,
    SETGLOBAL,
    SETTABLE,
    SETLIST,
    SETMAP,
    ADD,
    ADDI = 0x18,
    SUB,
    MULT,
    DIV,
    POW,
    CONCAT,
    MINUS,
    NOT,
    JMPNE = 0x20,
    JMPEQ,
    JMPLT,
    JMPLE,
    JMPGT,
    JMPGE,
    JMPT,
    JMPF,
    JMPONT = 0x28,
    JMPONF,
    JMP,
    PUSHNILJMP,
    FORPREP,
    FORLOOP,
    LFORPREP,
    LFORLOOP,
    CLOSURE = 0x30,
};

struct ChunkHeader
{
    bool is_little_endian;
    byte bytes_for_int;
    byte bytes_for_size_t;
    byte bytes_for_instruction;
    byte bits_for_instruction;
    byte bits_for_operator;
    byte bits_for_register_b;
};

struct Function
{
    String                  name;
    int                     line_defined;
    int                     number_of_params;
    bool                    is_variadic;
    int                     max_stack_size;
    Collection<Instruction> instructions;
    Collection<Number>      numbers;
    Collection<String>      globals;
    Collection<String>      locals;
    Collection<int>         lines;

    Collection<Function> functions;
};

struct Chunk
{
    ChunkHeader header;
    Function    main;
};

template<byte bits = 6>
Operator OP(Instruction instruction)
{
    return Operator(instruction & ((1 << bits) - 1));
}

template<byte bits = 15>
unsigned int A(Instruction instruction)
{
    return (instruction >> bits);
}

template<byte bits = 6>
unsigned int B(Instruction instruction)
{
    return (instruction >> bits);
}

template<byte bits = 6>
int S(Instruction instruction)
{
    return (instruction >> bits) - (std::numeric_limits<int>::max() >> bits);
}

// FSM and AST Definitions

struct Assignment;
struct Call;
struct ForLoop;
struct ForInLoop;
struct WhileLoop;
struct Condition;
using Statement = std::variant<Assignment, Call, ForLoop, ForInLoop, WhileLoop, Condition>;

void print_indent(const int indent);

struct AstNode
{
    virtual void print(const int indent = 0) = 0;
};

struct Assignment : AstNode
{
    std::string left;
    std::string right;

    void print(const int indent = 0) override
    {
        print_indent(indent);
        printf("%s = %s", left.c_str(), right.c_str());
    }
};

struct Call : AstNode
{
    std::string              name;
    std::vector<std::string> arguments;

    void print(const int indent = 0) override
    {
        print_indent(indent);
        printf("%s(", name.c_str());

        auto it = arguments.begin();
        while(it != arguments.end())
        {
            printf("%s", (*it).c_str());

            if(it != arguments.end() - 1)
                printf(", ");

            it++;
        }
        printf(")");
    }
};

struct ForLoop : AstNode
{
    std::string            begin;
    std::string            end;
    std::string            increment;
    std::vector<Statement> statements;

    void print(const int indent = 0) override;
};

struct ForInLoop : AstNode
{
    std::string            left = "temp";
    std::string            right;
    std::vector<Statement> statements;

    void print(const int indent = 0) override;
};

struct WhileLoop : AstNode
{
    std::string            condition;
    std::vector<Statement> statements;

    void print(const int indent = 0) override;
};

struct Condition : AstNode
{
    std::string            condition;
    std::vector<Statement> statements;

    void print(const int indent = 0) override;
};

struct Ast
{
    std::vector<std::string> stack;
    std::vector<Statement>   statements;
    Ast*                     body;
    Ast*                     parent;
};

struct Token
{
    Instruction instruction;
    Function*   function;
};

using TokenList       = std::vector<Token>;
using Action          = void (*)(Ast*&, const Token&, const Token&);
using TransitionTable = std::unordered_map<Operator, Action>;

void run_state_machine(Ast*& ast, const TokenList& tokens);
