#include "lua4dec.hpp"

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

    return buffer;
}

void create_ast(Ast*& ast, const char* filename)
{
    auto  buffer = read_file(filename);
    auto* iter   = buffer.data();
    auto  chunk  = read_chunk(iter);

    parse_function(ast, chunk.main);
}

void parse(const char* filename, FILE* stream)
{
    auto  buffer = read_file(filename);
    auto* iter   = buffer.data();
    auto  chunk  = read_chunk(iter);
    auto* ast    = new Ast();

    parse_function(ast, chunk.main);
    print_ast(*ast, stream);
}
