#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int main(int argc, char ** argv)
{
    if(argc < 4)
    {
        printf("Please provide path to compiler, decompiler, lua scripts.\n");
        return 1;
    }

    fs::path compiler(argv[1]);
    fs::path decompiler(argv[2]);
    fs::path scripts(argv[3]);

    std::string luac(compiler.u8string());
    std::string luadec(decompiler.u8string());

    const auto cwd = fs::current_path();

    printf("Start compiling ...\n");
    for (const auto& entry : fs::recursive_directory_iterator(scripts))
    {
        const auto path = fs::path(entry);
        const auto name = path.u8string();
        const auto ext = path.extension().u8string();

        if(ext.compare(".lua") == 0)
        {
            int res = system(luac.append(" ").append(name).append(" -o s.out").c_str());
            printf("%s %s\n", res == -1 ? "ERR" : "OK ", name.c_str());
        }
    }

    printf("Start decompiling ...\n");
    for (const auto& entry : fs::recursive_directory_iterator(scripts))
    {
        const auto path = fs::path(entry);
        const auto name = path.u8string();
        const auto ext = path.extension().u8string();

        if(ext.compare(".out") == 0)
        {
            int res = system(luadec.append(" ").append(name).c_str());
            printf("%s %s\n", res == -1 ? "ERR" : "OK ", name.c_str());
        }
    }

    return 0;
}