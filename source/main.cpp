#include "lua4dec.hpp"

int main(int argc, char** argv)
{
    Vector<Byte> buffer;

    if(argc < 2)
    {
        printf("Please provide a compiled lua script as argument.\n");
        return 1;
    }
    else if(argc == 2)
    {
#ifndef NDEBUG
        printf("Reading file: %s\n", argv[1]);
#endif
        buffer = read_file(argv[1]);
    }
    else if(argc > 3)
    {
        // pipe it here
        return 2;
    }

    auto* iter  = buffer.data();
    auto  chunk = read_chunk(iter);

#ifndef NDEBUG
    debug_chunk(chunk);
#endif

    auto* ast = new Ast();
    parse_function(State(), ast, chunk.main);

    print_ast(ast);

    if(argc == 3)
        write_file(argv[2], ast);

    return 0;
}
