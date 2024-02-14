#ifndef LUA4DEC_LUA_H
#define LUA4DEC_LUA_H

#include <assert.h>
#include <limits>
#include <set>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

using byte         = unsigned char;
using ByteIterator = byte*;
using String       = std::string;
template<typename T>
using Vector = std::vector<T>;

#ifndef TARGET_ARCH
#error "You must define a target architecture (32/64)"
#endif

#if TARGET_ARCH == 32
using Int         = int32_t;
using Instruction = uint32_t;
using SizeT       = uint32_t;
using Number      = float;
#else
using Int         = int32_t;
using Instruction = uint32_t;
using SizeT       = uint64_t;
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

extern std::unordered_map<Operator, std::string> OP_TO_STR;

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

struct Local
{
    String   name;
    unsigned start_pc;
    unsigned end_pc;
};

struct Function
{
    String              name;
    int                 line_defined;
    int                 number_of_params;
    bool                is_variadic;
    int                 max_stack_size;
    Vector<Instruction> instructions;
    Vector<Number>      numbers;
    Vector<String>      globals;
    Vector<Local>       locals;
    Vector<int>         lines;

    Vector<Function> functions;
};

struct Chunk
{
    ChunkHeader header;
    Function    main;
};

constexpr byte BITS_I      = sizeof(Instruction) * 8;
constexpr byte BITS_OP     = 6;
constexpr byte BITS_A      = 17;
constexpr byte BITS_B      = 9;  // TODO: must be defined at runtime
constexpr byte BITS_U      = 6;
constexpr byte BITS_S      = 6;
constexpr byte BIT_SHIFT_A = BITS_OP + BITS_B;
constexpr byte BIT_SHIFT_B = BITS_OP;
constexpr byte BIT_SHIFT_U = BITS_OP;
constexpr byte BIT_SHIFT_S = BITS_OP;

template<byte bits = BITS_OP>
Operator OP(Instruction instruction)
{
    return Operator(instruction & ((1 << bits) - 1));
}

template<byte shift = BIT_SHIFT_A, byte bits = BITS_A>
unsigned int A(Instruction instruction)
{
    return (instruction >> shift) & ((1 << bits) - 1);
}

template<byte shift = BIT_SHIFT_B, byte bits = BITS_B>
unsigned int B(Instruction instruction)
{
    return (instruction >> shift) & ((1 << bits) - 1);
}

template<byte shift = BIT_SHIFT_U>
unsigned int U(Instruction instruction)
{
    return (instruction >> shift);
}

template<byte shift = BIT_SHIFT_S>
int S(Instruction instruction)
{
    return (instruction >> shift) - (std::numeric_limits<int>::max() >> shift);
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

void debug_instruction(Instruction, Function&);
void debug_chunk(Chunk chunk);
void debug_function(Function function);

#endif  // LUA4DEC_LUA_H
