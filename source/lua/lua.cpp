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

    assert(header.bytes_for_int == sizeof(Int) && "Byte size for int mismatch");
    assert(header.bytes_for_size_t == sizeof(SizeT) && "Byte size for size_t mismatch");
    assert(header.bytes_for_instruction == BITS_I / 8 && "Byte size for instruction mismatch");
    assert(header.bits_for_instruction == BITS_I && "Bit size for instruction mismatch");
    assert(header.bits_for_operator == BITS_OP && "Bit size for operator mismatch");
    assert(header.bits_for_register_b == BITS_B && "Bit size for B mismatch");

    auto bytes_for_testnum = read<byte>(iter);
    assert(bytes_for_testnum == sizeof(Number) && "Bytes for test number mismatch");

    constexpr Number lua_number   = 3.14159265358979323846e8;
    const Number     magic_number = read<Number>(iter);
    const auto       diff         = lua_number - magic_number;
    assert(diff < 0.0000001 && "magic number mismatch");

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

void debug_instruction(Instruction instruction, Function& function)
{
    printf(
        "I: %11d (0x%08x) | OP: %2d (0x%02x) (%11s) "
        "| A: %5d (0x%07x) | B: %3d (0x%03x) | U: %10d (0x%08x) | S: %9d (0x%08x)",
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
    switch(OP(instruction))
    {
    case Operator::GETGLOBAL:
    case Operator::PUSHSTRING:
    case Operator::SETGLOBAL:
        name = function.globals[U(instruction)];
        break;
    case Operator::GETLOCAL:
    case Operator::SETLOCAL:
    {
        // TODO: This is a special hack for SWBF
        const auto pos = U(instruction);
        if(function.locals.size() > pos)
            name = function.locals[pos].name;
        else
            name = String("local" + std::to_string(pos));
        break;
    }
    case Operator::PUSHINT:
        name = std::to_string(S(instruction));
        break;
    case Operator::PUSHNUM:
    case Operator::PUSHNEGNUM:
        name = std::to_string(function.numbers[U(instruction)]);
        break;
        // TODO: Add the rest of the operators
    }

    if(!name.empty())
        printf(" | %s", name.c_str());

    printf("\n");
}

void debug_chunk(Chunk /*chunk*/)
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
    unsigned i = 0;
    for(const auto& g : function.globals)
    {
        printf("\t%d: \"%s\"\n", i++, g.c_str());
    }

    printf("Locals: %zu\n", function.locals.size());
    i = 0;
    for(const auto& l : function.locals)
    {
        printf("\t%d: \"%s\" (%u - %u)\n", i++, l.name.c_str(), l.start_pc, l.end_pc);
    }
    printf("\n");

    printf("Instructions: %zu\n", function.instructions.size());
    for(const auto& i : function.instructions)
    {
        debug_instruction(i, function);
    }
    printf("\n");

    for(const auto& fun : function.functions)
    {
        debug_function(fun);
    }
    printf("\n");
}
