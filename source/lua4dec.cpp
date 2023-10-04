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

    auto buffer     = Collection<byte>(len);
    auto bytes_read = (long)fread(buffer.data(), 1, len, stream);
    fclose(stream);

    if(len != bytes_read)
    {
        printf("Reading error %ld != %ld.\n", len, bytes_read);
        return {};
    }

    printf("Read %ld bytes from file.\n", len);
    fflush(stdout);

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

    auto* ast = new Ast();
    parse_function(ast, chunk.main);

    print_ast(*ast);

    return 0;
}
