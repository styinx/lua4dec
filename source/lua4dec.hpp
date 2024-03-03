#include "parser/parser.hpp"

Vector<Byte> read_file(const char* filename);
void         write_file(const char* filename, Ast const* const ast);
void         create_ast(Ast*& ast, const char* filename);
void         delete_ast(Ast*& ast);
void         parse(Ast*& ast, const char* filename, FILE* stream);
