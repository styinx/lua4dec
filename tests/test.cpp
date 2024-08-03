#include "lua4dec.hpp"

#include <array>
#include <filesystem>
#include <iostream>
#include <unordered_map>

namespace fs = std::filesystem;
using Buffer = std::vector<Byte>;
using Map    = std::unordered_map<std::string, std::array<Buffer, 2>>;

bool diff(Byte* first, Byte* second)
{
    while(first != nullptr || second != nullptr)
    {
        while(*first == ' ' || *first == '\t' || *first == '\n' || *first == '\r')
            first++;

        while(*second == ' ' || *second == '\t' || *second == '\n' || *second == '\r')
            second++;

        if(*first != *second)
            return false;

        first++;
        second++;
    }

    return true;
}

bool differ(const char* first, const char* second)
{
    auto buffer1 = read_file(first);
    auto buffer2 = read_file(second);

    return diff(buffer1.data(), buffer2.data());
}

int main(int argc, char** argv)
{
    constexpr char* ERR = "ERR";
    constexpr char* OK  = "OK ";
    Map             results;

    if(argc < 4)
    {
        printf("Provide path to compiler, decompiler, and lua scripts.\n");
        return 1;
    }

    fs::path compiler(argv[1]);
    fs::path decompiler(argv[2]);
    fs::path scripts(argv[3]);

    std::string luac(compiler.u8string());
    std::string luadec(decompiler.u8string());

    const auto cwd = fs::current_path();

    printf("Start compiling ...\n");
    for(const auto& entry : fs::recursive_directory_iterator(scripts))
    {
        const auto path = fs::path(entry);
        const auto file = path.u8string();
        const auto name = path.filename().u8string();
        const auto ext  = path.extension().u8string();

        if(ext.compare(".lua") == 0)
        {
            std::string cmd = luac;
            cmd.append(" -o ").append(file).append(".out ").append(file);

            results[name]    = {};
            results[name][0] = read_file(file.c_str());

            int res = system(cmd.c_str());
            printf("%s %s\n", res == 1 ? ERR : OK, file.c_str());
        }
    }

    printf("Start decompiling ...\n");
    for(const auto& entry : fs::recursive_directory_iterator(scripts))
    {
        const auto path = fs::path(entry);
        const auto file = path.u8string();
        const auto stem = path.stem().u8string();
        const auto ext  = path.extension().u8string();

        if(ext.compare(".out") == 0)
        {
            auto* ast   = new Ast();
            Status error = create_ast(ast, file.c_str());

            StringBuffer stream;
            print_ast(ast, stream);
            auto text = stream.str();

            results[stem][1] = Buffer(text.begin(), text.end());
            delete_ast(ast);

            printf("%s %s\n", error == Status::OK ? OK : ERR, file.c_str());
        }
    }

    printf("Start comparing ...\n");
    for(const auto& entry : fs::recursive_directory_iterator(scripts))
    {
        const auto path = fs::path(entry);
        const auto name = path.filename().u8string();
        const auto ext  = path.extension().u8string();

        if(ext.compare(".lua") == 0)
        {
            auto first   = results[name][0];
            auto second  = results[name][1];
            auto success = diff(first.data(), second.data());

            printf(
                "%s (%5lld B, %5lld B)  %s\n",
                success ? ERR : OK,
                first.size(),
                second.size(),
                name.c_str());
        }
    }

    return 0;
}
