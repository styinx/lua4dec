#include "parser/parser.hpp"

Vector<byte> read_file(const char* filename);
void         create_ast(Ast*& ast, const char* filename);
void         parse(const char* filename, FILE* stream);
