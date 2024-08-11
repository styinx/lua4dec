#include "errors.hpp"
#include "lua/lua.hpp"

String read_string(ByteIterator& iter)
{
    auto len = read<SizeT>(iter);
    auto str = String(iter, iter + len - 1);  // minus zero
    iter += len;
    return str;
}

String normalize(String&& str)
{
    auto pos = str.find('\n');
    if(pos != std::string::npos)
    {
        str[pos] = ' ';
    }
    return str;
}

ChunkHeader read_header(ByteIterator& iter)
{
    ChunkHeader header;

    // Read signature
    bool signature_ok = true;
    signature_ok &= read<Byte>(iter) == 0x1B;  // . (ESC)
    signature_ok &= read<Byte>(iter) == 0x4C;  // L
    signature_ok &= read<Byte>(iter) == 0x75;  // u
    signature_ok &= read<Byte>(iter) == 0x61;  // a
    signature_ok &= read<Byte>(iter) == 0x40;  // @ (4.0)

    quit_on(!signature_ok, Status::SIGNATURE_MISMATCH, "Header mismatch! '.Lua@' not found");

    // Read size of types, registers, and the test number
    header.is_little_endian      = read<Byte>(iter) == 0x01;
    header.bytes_for_int         = read<Byte>(iter);
    header.bytes_for_size_t      = read<Byte>(iter);
    header.bytes_for_instruction = read<Byte>(iter);
    header.bits_for_instruction  = read<Byte>(iter);
    header.bits_for_operator     = read<Byte>(iter);
    header.bits_for_register_b   = read<Byte>(iter);
    header.bytes_for_test_number = read<Byte>(iter);
    header.test_number           = read<Number>(iter);

    bool architecture_ok = true;
    architecture_ok &= header.bytes_for_int == sizeof(Int);
    architecture_ok &= header.bytes_for_size_t == sizeof(SizeT);
    architecture_ok &= header.bytes_for_instruction == BITS_I / 8;
    architecture_ok &= header.bits_for_instruction == BITS_I;
    architecture_ok &= header.bits_for_operator == BITS_OP;
    architecture_ok &= header.bits_for_register_b == BITS_B;
    architecture_ok &= header.bytes_for_test_number == sizeof(Number);
    architecture_ok &= (LUA_NUMBER - header.test_number) < 0.0000001;

    quit_on(
        !architecture_ok,
        Status::ARCHITECTURE_MISMATCH,
        "Architecture mismatch! (32 bit <-> 64 bit)");

    return header;
}

Function read_function(ByteIterator& iter)
{
    Function function;

    function.name             = read_string(iter);
    function.line_defined     = read<int>(iter);
    function.number_of_params = read<int>(iter);
    function.is_variadic      = read<Byte>(iter) == 0x01;
    function.max_stack_size   = read<int>(iter);

    auto num_locals = read<int>(iter);
    for(int i = 0; i < num_locals; i++)
    {
        Local local;
        local.name     = read_string(iter);
        local.start_pc = read<int>(iter);
        local.end_pc   = read<int>(iter);
        function.locals.emplace_back(local);
    }

    auto num_lineinfo = read<int>(iter);
    for(int i = 0; i < num_lineinfo; i++)
    {
        function.lines.emplace_back(read<int>(iter));
    }

    auto num_constants = read<int>(iter);
    for(int i = 0; i < num_constants; i++)
    {
        function.globals.emplace_back(normalize(read_string(iter)));
    }

    auto num_numbers = read<int>(iter);
    for(int i = 0; i < num_numbers; i++)
    {
        function.numbers.emplace_back(read<Number>(iter));
    }

    auto num_functions = read<int>(iter);
    for(int i = 0; i < num_functions; i++)
    {
        function.functions.emplace_back(read_function(iter));
    }

    auto num_instructions = read<int>(iter);
    for(int i = 0; i < num_instructions; i++)
    {
        function.instructions.emplace_back(read<Instruction>(iter));
    }

    return function;
}

Chunk read_chunk(ByteIterator& iter)
{
    Chunk chunk;

    chunk.header = read_header(iter);
    chunk.main   = read_function(iter);

    return chunk;
}

// clang-format off
std::unordered_map<Operator, std::string> OP_TO_STR = {
    {Operator::END,         "END"},
    {Operator::RETURN,      "RETURN"},
    {Operator::CALL,        "CALL"},
    {Operator::TAILCALL,    "TAILCALL"},
    {Operator::PUSHNIL,     "PUSHNIL"},
    {Operator::POP,         "POP"},
    {Operator::PUSHINT,     "PUSHINT"},
    {Operator::PUSHSTRING,  "PUSHSTRING"},
    {Operator::PUSHNUM,     "PUSHNUM"},
    {Operator::PUSHNEGNUM,  "PUSHNEGNUM"},
    {Operator::PUSHUPVALUE, "PUSHUPVALUE"},
    {Operator::GETLOCAL,    "GETLOCAL"},
    {Operator::GETGLOBAL,   "GETGLOBAL"},
    {Operator::GETTABLE,    "GETTABLE"},
    {Operator::GETDOTTED,   "GETDOTTED"},
    {Operator::GETINDEXED,  "GETINDEXED"},
    {Operator::PUSHSELF,    "PUSHSELF"},
    {Operator::CREATETABLE, "CREATETABLE"},
    {Operator::SETLOCAL,    "SETLOCAL"},
    {Operator::SETGLOBAL,   "SETGLOBAL"},
    {Operator::SETTABLE,    "SETTABLE"},
    {Operator::SETLIST,     "SETLIST"},
    {Operator::SETMAP,      "SETMAP"},
    {Operator::ADD,         "ADD"},
    {Operator::ADDI,        "ADDI"},
    {Operator::SUB,         "SUB"},
    {Operator::MULT,        "MULT"},
    {Operator::DIV,         "DIV"},
    {Operator::POW,         "POW"},
    {Operator::CONCAT,      "CONCAT"},
    {Operator::MINUS,       "MINUS"},
    {Operator::NOT,         "NOT"},
    {Operator::JMPNE,       "JMPNE"},
    {Operator::JMPEQ,       "JMPEQ"},
    {Operator::JMPLT,       "JMPLT"},
    {Operator::JMPLE,       "JMPLE"},
    {Operator::JMPGT,       "JMPGT"},
    {Operator::JMPGE,       "JMPGE"},
    {Operator::JMPT,        "JMPT"},
    {Operator::JMPF,        "JMPF"},
    {Operator::JMPONT,      "JMPONT"},
    {Operator::JMPONF,      "JMPONF"},
    {Operator::JMP,         "JMP"},
    {Operator::PUSHNILJMP,  "PUSHNILJMP"},
    {Operator::FORPREP,     "FORPREP"},
    {Operator::FORLOOP,     "FORLOOP"},
    {Operator::LFORPREP,    "LFORPREP"},
    {Operator::LFORLOOP,    "LFORLOOP"},
    {Operator::CLOSURE,     "CLOSURE"},
};
// clang-format on

void debug_chunk(Chunk chunk)
{
    DebugState state;

    debug_header(chunk.header);
    debug_function(state, chunk.main);
}

void debug_header(ChunkHeader header)
{
    printf("=== Chunk Header ===\n");
    printf("Endianness:            %s\n", header.is_little_endian ? "little" : "big");
    printf("Bytes for int:         %2u B\n", header.bytes_for_int);
    printf("Bytes for size_t:      %2u B\n", header.bytes_for_size_t);
    printf("Bytes for instruction: %2u B\n", header.bytes_for_instruction);
    printf("Bits for instruction:  %2u bits\n", header.bits_for_instruction);
    printf("Bits for operator:     %2u bits\n", header.bits_for_operator);
    printf("Bits for register B:   %2u bits\n", header.bits_for_register_b);
    printf("Bytes for test number: %2u B\n", header.bytes_for_test_number);
    printf("Test number:           %1.16e\n\n", header.test_number);
}

void debug_function(DebugState& state, Function function)
{
    printf("=== Function ===\n");
    printf("Name:         \"%s\"\n", function.name.c_str());
    printf("Line:         %d\n", function.line_defined);
    printf("Params:       %d\n", function.number_of_params);
    printf("Variadic:     %s\n", function.is_variadic ? "true" : "false");
    printf("Stack:        %d\n", function.max_stack_size);

    printf("Locals:       %zu\n", function.locals.size());
    unsigned n = 0;
    for(const auto& l : function.locals)
    {
        printf(" %3d: \"%s\" (%u - %u)\n", n++, l.name.c_str(), l.start_pc, l.end_pc);
    }

    printf("Globals:      %zu\n", function.globals.size());
    n = 0;
    for(const auto& g : function.globals)
    {
        printf(" %3d: \"%s\"\n", n++, g.c_str());
    }

    printf("Instructions: %zu\n", function.instructions.size());
    n = 0;
    for(const auto& i : function.instructions)
    {
        debug_instruction(state, n, i, function);
        n++;

        state.PC++;
    }
    printf("\n");

    for(const auto& fun : function.functions)
    {
        auto new_state = DebugState();
        debug_function(new_state, fun);
    }
}

void debug_instruction(DebugState& state, unsigned idx, Instruction instruction, Function& function)
{
    printf(
        " %3d: %11d (0x%08x) | OP: %2d (0x%02x) (%11s) | "
        "A: %5d (0x%07x) | B: %3d (0x%03x) | U: %10d (0x%08x) | S: %9d (0x%08x)",
        idx,
        (int)instruction,
        (int)instruction,
        (int)OP(instruction),
        (int)OP(instruction),
        OP_TO_STR[OP(instruction)].c_str(),
        A(instruction),
        A(instruction),
        B(instruction),
        B(instruction),
        U(instruction),
        U(instruction),
        S(instruction),
        S(instruction));

    std::string name;
    bool        check_emptyness = true;
    switch(OP(instruction))
    {
    case Operator::CALL:
    case Operator::TAILCALL:
        break;
    case Operator::GETGLOBAL:
    case Operator::PUSHSTRING:
    case Operator::SETGLOBAL:
        name = function.globals[U(instruction)];
        break;
    case Operator::GETLOCAL:
    case Operator::SETLOCAL:
    {
        const auto pos = U(instruction);

        auto index = 0;
        auto i     = 0;
        while(i != pos)
        {
            if(function.locals[index].start_pc <= state.PC &&
               function.locals[index].end_pc >= state.PC)
                i++;
            index++;
        }

        name = function.locals[index].name;
        break;
    }
    case Operator::PUSHINT:
        name = std::to_string(S(instruction));
        break;
    case Operator::PUSHNUM:
    case Operator::PUSHNEGNUM:
        name = std::to_string(function.numbers[U(instruction)]);
        break;
    default:
        check_emptyness = false;
        // TODO: Add the rest of the operators
    }

    if(check_emptyness && name.empty())
        name = "\"\"";

    if(!name.empty())
        printf(" | %s", name.c_str());

    printf("\n");
}

void quit_on(const bool condition, const Status error, const char* message)
{
    if(condition)
    {
        printf("%s\n", message);
        exit(static_cast<int>(error));
    }
}
