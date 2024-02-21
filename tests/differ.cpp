#include "lua4dec.hpp"

#include <stdio.h>

bool diff(byte* first, byte* second)
{
    while(first != nullptr || second != nullptr)
    {
        printf("%c %c\n", *first, *second);
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
    if(argc < 2)
    {
        printf("Provide at least 2 files as arguments.\n");
        return 1;
    }
    return differ(argv[1], argv[2]) ? 0 : 1;
}
