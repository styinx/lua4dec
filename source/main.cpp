#include "lua4dec.hpp"

int main(int argc, char** argv)
{
    Vector<Byte> buffer;

    if(argc < 2)
    {
        printf("Please provide a compiled lua script as argument.\n");
        return 1;
    }
    else if(argc > 1)
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

    auto* ast    = new Ast();
    auto  state  = State();
    auto  result = parse_function(state, ast, chunk.main);

    if(result == Status::OK)
    {
#ifndef NDEBUG
        print_ast(ast);
#endif

        if(argc > 2)
        {
#ifndef NDEBUG
        printf("Writing file: %s\n", argv[1]);
#endif
            write_file(argv[2], ast);
        }
    }

#ifndef NDEBUG
    printf("Exit Code: %d (%s)", result, STATUS_TO_STR[result].c_str());
#endif

    return static_cast<unsigned>(result);
}
