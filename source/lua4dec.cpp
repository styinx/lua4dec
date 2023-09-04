#include "parser/parser.hpp"

Collection<byte> read_file(const char * filename)
{
    auto* file = fopen(filename, "rb");

    if(file == nullptr)
    {
        return {};
    }

    fseek(file, 0, SEEK_END);
    auto len = ftell(file);
    fseek(file, 0, SEEK_SET);

    auto buffer = Collection<byte>(len);
    fread(buffer.data(), 1, len, file);
    fclose(file);

    return buffer;
}

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        printf("Please provide a compiled lua script as argument.\n");
    }

    auto buffer = read_file(argv[1]);

    auto* iter = buffer.data();
    auto chunk = read_chunk(iter);

    debug_chunk(chunk);
    debug_function(chunk.main);

    auto  tokens = parse_bytecode(chunk.main);
    auto* ast    = new Ast();
    run_state_machine(ast, tokens);

    print_ast(*ast);

    return 0;
}
