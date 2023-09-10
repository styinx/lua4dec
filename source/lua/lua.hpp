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

static constexpr unsigned MAX_INT = 2147483647 - 2;

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

/*
 * Read bytecode
 */

template<typename T>
T read(ByteIterator& iter, bool advance = true)
{
    T element = *reinterpret_cast<T*>(iter);
    iter += (advance) ? sizeof(T) : 0;
    return element;
}

template<size_t n>
ByteIterator readn(ByteIterator& iter, bool advance = true)
{
    ByteIterator bytes = iter;
    iter += (advance) ? n : 0;
    return bytes;
}

String      read_string(ByteIterator&);
String      normalize(String&&);
ChunkHeader read_header(ByteIterator&);
Function    read_function(ByteIterator&);
Chunk       read_chunk(ByteIterator&);

void debug_instruction(Instruction);
void debug_chunk(Chunk chunk);
void debug_function(Function function);
