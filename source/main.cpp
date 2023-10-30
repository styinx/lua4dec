#include "lua4dec.hpp"

int main(int argc, char** argv)
{
    Collection<byte> buffer;

    if(argc < 2)
    {
        printf("Please provide a compiled lua script as argument.\n");
        return 1;
    }
    else if(argc == 2)
    {
        buffer = read_file(argv[1]);
    }
    else
    {
        // pipe it here
        return 2;
    }

    auto* iter  = buffer.data();
    auto  chunk = read_chunk(iter);

#ifndef NDEBUG
    debug_chunk(chunk);
    debug_function(chunk.main);
#endif

    auto* ast = new Ast();
    parse_function(ast, chunk.main);

    print_ast(*ast);

    return 0;
}
