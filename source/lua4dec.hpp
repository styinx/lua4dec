#include "parser/parser.hpp"

Vector<byte> read_file(const char* filename);
void         create_ast(Ast*& ast, const char* filename);
void         delete_ast(Ast*& ast);
void         parse(Ast*& ast, const char* filename, FILE* stream);
