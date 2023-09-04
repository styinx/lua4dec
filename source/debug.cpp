#include <stdio.h>

#include "debug.h"

void debug_instruction(Instruction instruction)
{
	printf("I: %12d (0x%08x)  OP: %2d (0x%02x)\n",
           (int)instruction, (int)instruction, OP(instruction), OP(instruction));
}

void debug_chunk(Chunk chunk)
{
   //printf("%s\n", chunk.name); 
}

void debug_function(Function function)
{
   printf("Name: %s\n", function.name.c_str()); 
   printf("Line: %d\n", function.line_defined); 
   printf("Params: %d\n", function.number_of_params); 
   printf("Variadic: %s\n", function.is_variadic ? "true" : "false"); 
   printf("Stack: %d\n", function.max_stack_size); 

   printf("Globals: %zu\n\t", function.globals.size()); 
   for(const auto & global : function.globals)
   {
        printf("\"%s\", ", global.c_str());
   }
   printf("\n");
}
