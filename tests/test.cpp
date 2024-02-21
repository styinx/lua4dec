#include "lua4dec.hpp"

#include <filesystem>
#include <iostream>
#include <unordered_map>

namespace fs = std::filesystem;
using Buffer = std::vector<byte>;
using Map    = std::unordered_map<std::string, std::pair<Buffer, Buffer>>;

int main(int argc, char** argv)
{
    constexpr char* ERR = "ERR";
    constexpr char* OK  = "OK ";
    Map             results;

    if(argc < 5)
    {
        printf("Provide path to compiler, decompiler, differ, and lua scripts.\n");
        return 1;
    }

    fs::path compiler(argv[1]);
    fs::path decompiler(argv[2]);
    fs::path differ(argv[3]);
    fs::path scripts(argv[4]);

    std::string luac(compiler.u8string());
    std::string luadec(decompiler.u8string());

    const auto cwd = fs::current_path();

    printf("Start compiling ...\n");
    for(const auto& entry : fs::recursive_directory_iterator(scripts))
    {
        const auto path = fs::path(entry);
        const auto name = path.u8string();
        const auto ext  = path.extension().u8string();

        if(ext.compare(".lua") == 0)
        {
            std::string cmd = luac;
            cmd.append(" -o ").append(name).append(".out ").append(name);

            results[name] = {};
            // results[name][0] = read_file(name.c_str());

            int res = system(cmd.c_str());
            printf("%s %s\n", res == 1 ? ERR : OK, name.c_str());
        }
    }

    printf("Start decompiling ...\n");
    for(const auto& entry : fs::recursive_directory_iterator(scripts))
    {
        const auto path = fs::path(entry);
        const auto name = path.u8string();
        const auto ext  = path.extension().u8string();

        if(ext.compare(".out") == 0)
        {
            std::string cmd = luac;
            cmd.append(" ").append(name);

            auto* ast = new Ast();
            create_ast(ast, name.c_str());
            // TODO
            // results[name][1] = read_file(name.c_str());

            int res = system(cmd.c_str());
            printf("%s %s\n", res == -1 ? ERR : OK, name.c_str());
        }
    }

    printf("Start comparing ...\n");
    for(const auto& entry : fs::recursive_directory_iterator(scripts))
    {
        const auto path = fs::path(entry);
        const auto name = path.u8string();
        const auto ext  = path.extension().u8string();

        if(ext.compare(".out") == 0)
        {
            // TODO
            std::string cmd = differ;
            cmd.append(" ").append(name);

            int res = system(cmd.c_str());
            printf("%s %s\n", res == 1 ? ERR : OK, name.c_str());
        }
    }

    return 0;
}
