#include "parser/parser.hpp"

Collection<byte> read_file(const char* filename)
{
    auto* stream = fopen(filename, "rb");

    if(stream == nullptr)
    {
        return {};
    }

    fseek(stream, 0, SEEK_END);
    auto len = ftell(stream);
    fseek(stream, 0, SEEK_SET);

    printf("%ld", len);
    fflush(stdout);

    auto buffer = Collection<byte>(len);
    fread(buffer.data(), 1, len, stream);
    fclose(stream);

    return buffer;
}

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

    debug_chunk(chunk);
    debug_function(chunk.main);

    auto  tokens = parse_bytecode(chunk.main);
    auto* ast    = new Ast();
    run_parser_machine(ast, tokens);

    print_ast(*ast);

    return 0;
}
