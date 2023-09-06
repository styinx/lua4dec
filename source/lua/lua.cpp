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
        str[pos] = '_';
    }
    return str;
}

ChunkHeader read_header(ByteIterator& iter)
{
    ChunkHeader header;

    // Read signature
    assert(read<byte>(iter) == 0x1B && "Header mismatch");  // .
    assert(read<byte>(iter) == 0x4C && "Header mismatch");  // L
    assert(read<byte>(iter) == 0x75 && "Header mismatch");  // u
    assert(read<byte>(iter) == 0x61 && "Header mismatch");  // a
    assert(read<byte>(iter) == 0x40 && "Header mismatch");  // @ (4.0)

    // Read size of types and registers
    header.is_little_endian      = read<byte>(iter) == 0x01;
    header.bytes_for_int         = read<byte>(iter);
    header.bytes_for_size_t      = read<byte>(iter);
    header.bytes_for_instruction = read<byte>(iter);
    header.bits_for_instruction  = read<byte>(iter);
    header.bits_for_operator     = read<byte>(iter);
    header.bits_for_register_b   = read<byte>(iter);

    auto bytes_for_testnum = read<byte>(iter);

    assert(sizeof(SizeT) == header.bytes_for_size_t && "size_t mismatch");

    assert(3.14159265358979323846e8 - read<Number>(iter) < 0.0000001 && "magic mismatch");

    return header;
}

Function read_function(ByteIterator& iter)
{
    Function function;

    function.name             = read_string(iter);
    function.line_defined     = read<int>(iter);
    function.number_of_params = read<int>(iter);
    function.is_variadic      = read<byte>(iter) == 0x01;
    function.max_stack_size   = read<int>(iter);

    auto num_locals = read<int>(iter);
    for(int i = 0; i < num_locals; i++)
    {
        function.locals.emplace_back(read_string(iter));
        auto start_pc = read<int>(iter);
        auto end_pc   = read<int>(iter);
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

std::unordered_map<Operator, std::string> OP_TO_STR = {
    {Operator::END, "END"},
    {Operator::RETURN, "RETURN"},
    {Operator::CALL, "CALL"},
    {Operator::TAILCALL, "TAILCALL"},
    {Operator::PUSHNIL, "PUSHNIL"},
    {Operator::POP, "POP"},
    {Operator::PUSHINT, "PUSHINT"},
    {Operator::PUSHSTRING, "PUSHSTRING"},
    {Operator::PUSHNUM, "PUSHNUM"},
    {Operator::PUSHNEGNUM, "PUSHNEGNUM"},
    {Operator::PUSHUPVALUE, "PUSHUPVALUE"},
    {Operator::GETLOCAL, "GETLOCAL"},
    {Operator::GETGLOBAL, "GETGLOBAL"},
    {Operator::GETTABLE, "GETTABLE"},
    {Operator::GETDOTTED, "GETDOTTED"},
    {Operator::GETINDEXED, "GETINDEXED"},
    {Operator::PUSHSELF, "PUSHSELF"},
    {Operator::CREATETABLE, "CREATETABLE"},
    {Operator::SETLOCAL, "SETLOCAL"},
    {Operator::SETGLOBAL, "SETGLOBAL"},
    {Operator::SETTABLE, "SETTABLE"},
    {Operator::SETLIST, "SETLIST"},
    {Operator::SETMAP, "SETMAP"},
    {Operator::ADD, "ADD"},
    {Operator::ADDI, "ADDI"},
    {Operator::SUB, "SUB"},
    {Operator::MULT, "MULT"},
    {Operator::DIV, "DIV"},
    {Operator::POW, "POW"},
    {Operator::CONCAT, "CONCAT"},
    {Operator::MINUS, "MINUS"},
    {Operator::NOT, "NOT"},
    {Operator::JMPNE, "JMPNE"},
    {Operator::JMPEQ, "JMPEQ"},
    {Operator::JMPLT, "JMPLT"},
    {Operator::JMPLE, "JMPLE"},
    {Operator::JMPGT, "JMPGT"},
    {Operator::JMPGE, "JMPGE"},
    {Operator::JMPT, "JMPT"},
    {Operator::JMPF, "JMPF"},
    {Operator::JMPONT, "JMPONT"},
    {Operator::JMPONF, "JMPONF"},
    {Operator::JMP, "JMP"},
    {Operator::PUSHNILJMP, "PUSHNILJMP"},
    {Operator::FORPREP, "FORPREP"},
    {Operator::FORLOOP, "FORLOOP"},
    {Operator::LFORPREP, "LFORPREP"},
    {Operator::LFORLOOP, "LFORLOOP"},
    {Operator::CLOSURE, "CLOSURE"},
};

void debug_instruction(Instruction instruction)
{
    printf(
        "I: 0x%08x (%11d)  A: 0x%08x (%11d)  B: 0x%08x (%11d)  OP: %2d (0x%02x) (%s)\n",
        (int)instruction,
        (int)instruction,
        A(instruction),
        A(instruction),
        B(instruction),
        B(instruction),
        OP(instruction),
        OP(instruction),
        OP_TO_STR[OP(instruction)].c_str());
}

void debug_chunk(Chunk chunk)
{
    // printf("%s\n", chunk.name);
}

void debug_function(Function function)
{
    printf("Name: %s\n", function.name.c_str());
    printf("Line: %d\n", function.line_defined);
    printf("Params: %d\n", function.number_of_params);
    printf("Variadic: %s\n", function.is_variadic ? "true" : "false");
    printf("Stack: %d\n", function.max_stack_size);

    printf("Globals: %zu\n", function.globals.size());
    for(int i = 0; i < function.globals.size(); ++i)
    {
        printf("\t\"%s\"\n", function.globals[i].c_str());
    }
    printf("\n");
}

