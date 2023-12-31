#include "parser/parser.hpp"

#include <algorithm>

unsigned PC = 0;

// Get a local variable that is defined at the current PC offset.
// Use the index to get the nth next variable.
String get_local_from_pc(const Function& function, unsigned pc_offset, unsigned index = 0)
{
    for(unsigned i = 0; i < function.locals.size(); ++i)
    {
        if(function.locals[i].start_pc >= pc_offset)
            if(i + index < function.locals.size())
                return function.locals[i + index].name;
    }
    return "";
}

void enter_block(Ast*&, const Instruction&, const Function&);
void exit_block(Ast*&);
void empty(Ast*&, const Instruction&, const Function&);
void push_nil(Ast*&, const Instruction&, const Function&);
void push_int(Ast*&, const Instruction&, const Function&);
void push_string(Ast*&, const Instruction&, const Function&);
void push_num(Ast*&, const Instruction&, const Function&);
void push_local(Ast*&, const Instruction&, const Function&);
void push_global(Ast*&, const Instruction&, const Function&);
void push_dotted(Ast*&, const Instruction&, const Function&);
void push_indexed(Ast*&, const Instruction&, const Function&);
void push_self(Ast*&, const Instruction&, const Function&);
void push_table(Ast*&, const Instruction&, const Function&);
void push_list(Ast*&, const Instruction&, const Function&);
void push_map(Ast*&, const Instruction&, const Function&);
void pop(Ast*&, const Instruction&, const Function&);
void make_add(Ast*&, const Instruction&, const Function&);
void make_addi(Ast*&, const Instruction&, const Function&);
void make_sub(Ast*&, const Instruction&, const Function&);
void make_mult(Ast*&, const Instruction&, const Function&);
void make_div(Ast*&, const Instruction&, const Function&);
void make_pow(Ast*&, const Instruction&, const Function&);
void make_concat(Ast*&, const Instruction&, const Function&);
void make_minus(Ast*&, const Instruction&, const Function&);
void make_not(Ast*&, const Instruction&, const Function&);
void make_call(Ast*&, const Instruction&, const Function&);
void make_tail_call(Ast*&, const Instruction&, const Function&);
void make_local_assignment(Ast*&, const Instruction&, const Function&);
void make_assignment(Ast*&, const Instruction&, const Function&);
void make_table_assignment(Ast*&, const Instruction&, const Function&);
void enter_for_loop(Ast*&, const Instruction&, const Function&);
void make_for_loop(Ast*&, const Instruction&, const Function&);
void make_for_in_loop(Ast*&, const Instruction&, const Function&);
void make_condition(Ast*&, const Instruction&, const Function&);
void end_condition(Ast*&, const Instruction&, const Function&);
void make_closure(Ast*&, const Instruction&, const Function&);

// clang-format off
auto TABLE = ActionTable
{
    {Operator::END,         &empty},
    {Operator::RETURN,      &empty}, // TODO
    // Stack modification
    {Operator::PUSHNIL,     &push_nil},
    {Operator::POP,         &pop},
    {Operator::PUSHINT,     &push_int},
    {Operator::PUSHSTRING,  &push_string},
    {Operator::PUSHNUM,     &push_num},
    {Operator::PUSHNEGNUM,  &push_num},
    {Operator::PUSHUPVALUE, &empty}, // TODO
    {Operator::GETLOCAL,    &push_local},
    {Operator::GETGLOBAL,   &push_global},
    {Operator::GETTABLE,    &empty}, // TODO
    {Operator::GETDOTTED,   &push_dotted},
    {Operator::GETINDEXED,  &push_indexed},
    {Operator::PUSHSELF,    &push_self},
    {Operator::CREATETABLE, &push_table},
    {Operator::SETLIST,     &push_list},
    {Operator::SETMAP,      &push_map},
    {Operator::PUSHNILJMP,  &empty}, // TODO
    // Operations
    {Operator::ADD,         &make_add},
    {Operator::ADDI,        &make_addi},
    {Operator::SUB,         &make_sub},
    {Operator::MULT,        &make_mult},
    {Operator::DIV,         &make_div},
    {Operator::POW,         &make_pow},
    {Operator::CONCAT,      &make_concat},
    {Operator::MINUS,       &make_minus},
    {Operator::NOT,         &make_not},
    // Call
    {Operator::CALL,        &make_call},
    {Operator::TAILCALL,    &make_tail_call},
    // Assignment
    {Operator::SETLOCAL,    &make_local_assignment},
    {Operator::SETGLOBAL,   &make_assignment},
    {Operator::SETTABLE,    &make_table_assignment},
    // For loop
    {Operator::FORPREP,     &enter_for_loop},
    {Operator::FORLOOP,     &make_for_loop},
    // For in loop
    {Operator::LFORPREP,    &enter_for_loop},
    {Operator::LFORLOOP,    &make_for_in_loop},
    // Conditions
    {Operator::JMPNE,       &make_condition},
    {Operator::JMPEQ,       &make_condition},
    {Operator::JMPLT,       &make_condition},
    {Operator::JMPLE,       &make_condition},
    {Operator::JMPGT,       &make_condition},
    {Operator::JMPGE,       &make_condition},
    {Operator::JMPT,        &make_condition},
    {Operator::JMPF,        &make_condition},
    {Operator::JMPONT,      &make_condition},
    {Operator::JMPONF,      &make_condition},
    {Operator::JMP,         &end_condition},
    // Closure
    {Operator::CLOSURE,     &make_closure},
};

// clang-format on

// Block management

void enter_block(Ast*& ast)
{
    auto* child   = new Ast();
    child->parent = ast;

    ast->child = child;
    ast        = child;
}

void enter_block(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    auto* child   = new Ast();
    child->parent = ast;

    ast->child = child;
    ast        = child;
}

void exit_block(Ast*& ast)
{
    if(ast->parent)
        ast = ast->parent;
}

// Helpers

void empty(Ast*&, const Instruction&, const Function&)
{
}

// Stack modification

void push_nil(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    ast->stack.push_back(Identifier("nil"));
}

void pop(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    ast->stack.pop_back();
}

void push_global(Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto name = function.globals[U(instruction)];
    ast->stack.push_back(Identifier(name));
}

void push_dotted(Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto name   = function.globals[U(instruction)];
    const auto parent = std::get<Identifier>(std::get<Expression>(ast->stack.back()));
    ast->stack.pop_back();
    ast->stack.push_back(Identifier(parent.name + "." + name));
}

void push_indexed(Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto name   = function.locals[U(instruction)].name;
    const auto parent = std::get<Identifier>(std::get<Expression>(ast->stack.back()));
    ast->stack.pop_back();
    ast->stack.push_back(Identifier(parent.name + "[" + name + "]"));
}

void push_self(Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto name   = function.globals[U(instruction)];
    const auto parent = std::get<Identifier>(std::get<Expression>(ast->stack.back()));
    ast->stack.push_back(Identifier(parent.name + "." + name));
}

void push_local(Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto name = function.locals[U(instruction)].name;
    ast->stack.push_back(Identifier(name));
}

void push_int(Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    const auto value = S(instruction);
    ast->stack.push_back(AstInt(value));
}

void push_num(Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto value = function.numbers[U(instruction)];
    ast->stack.push_back(AstNumber(value));
}

void push_string(Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto value = function.globals[U(instruction)];
    ast->stack.push_back(AstString(value));
}

void push_table(Ast*& ast, const Instruction& instruction, const Function& function)
{
    // Its only a table if an identifier is on the stack before. Otherwise its a map or
    // list.

    std::string name;
    if(ast->stack.size())
    {
        const auto ex = std::get<Expression>(ast->stack.back());
        if(std::holds_alternative<Identifier>(ex))
        {
            name = std::get<Identifier>(ex).name;
            ast->stack.pop_back();
        }
    }

    const auto size = B(instruction);
    AstTable   table(size, name, Collection<std::pair<Expression, Expression>>{});
    ast->stack.push_back(table);
}

void push_list(Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    Collection<Expression> list;
    for(unsigned i = 0; i < B(instruction); ++i)
    {
        list.push_back(std::get<Expression>(ast->stack.back()));
        ast->stack.pop_back();
    }

    std::reverse(list.begin(), list.end());

    ast->stack.pop_back();  // empty AstTable
    ast->stack.push_back(AstList(list));
}

void push_map(Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    Collection<std::pair<Expression, Expression>> map;
    for(unsigned i = 0; i < U(instruction); ++i)
    {
        const auto value = std::get<Expression>(ast->stack.back());
        ast->stack.pop_back();

        const auto key = std::get<Expression>(ast->stack.back());
        ast->stack.pop_back();

        map.push_back(std::make_pair(key, value));
    }

    std::reverse(map.begin(), map.end());

    auto table = std::get<AstTable>(std::get<Expression>(ast->stack.back()));
    if(table.name.name.empty())
        ast->stack.pop_back();  // empty AstTable

    ast->stack.push_back(AstMap(map));
}

// Operations
void make_add(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto left = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    const auto right = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    ast->stack.push_back(AstOperation("+", {left, right}));
}

void make_addi(Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    const auto left = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    const auto right = AstNumber(S(instruction));
    ast->stack.push_back(AstOperation("+=", {left, right}));
}

void make_sub(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto left = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    const auto right = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    ast->stack.push_back(AstOperation("-", {left, right}));
}

void make_mult(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto left = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    const auto right = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    ast->stack.push_back(AstOperation("*", {left, right}));
}

void make_div(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto left = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    const auto right = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    ast->stack.push_back(AstOperation("/", {left, right}));
}

void make_pow(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto left = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    const auto right = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    ast->stack.push_back(AstOperation("^", {left, right}));
}

void make_concat(Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    Collection<Expression> ex;
    for(unsigned i = 0; i < U(instruction); ++i)
    {
        ex.push_back(std::get<Expression>(ast->stack.back()));
        ast->stack.pop_back();
    }
    ast->stack.push_back(AstOperation("..", ex));
}

void make_minus(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto right = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    ast->stack.push_back(AstOperation("-", {right}));
}

void make_not(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto right = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    ast->stack.push_back(AstOperation("not", {right}));
}

// Assignment

void make_assignment(Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto left  = Identifier(function.globals[B(instruction)]);
    const auto right = std::get<Expression>(ast->stack.back());
    Assignment ass(left, right);
    ast->stack.pop_back();

    ast->statements.push_back(ass);
}

void make_local_assignment(Ast*& ast, const Instruction& instruction, const Function& function)
{
    const auto left  = Identifier(function.locals[B(instruction)].name);
    const auto right = std::get<Expression>(ast->stack.back());
    Assignment ass(left, right);
    ast->stack.pop_back();

    ast->statements.push_back(ass);
}

void make_table_assignment(Ast*& ast, const Instruction& instruction, const Function& function)
{
    Collection<Expression> args;
    for(unsigned i = 0; i < B(instruction); ++i)
    {
        args.push_back(std::get<Expression>(ast->stack.back()));
        ast->stack.pop_back();
    }

    std::reverse(args.begin(), args.end());

    std::string left;
    for(auto it = args.begin(); it != args.end() - 1; ++it)
    {
        if(std::holds_alternative<Identifier>(*it))
            left.append(std::get<Identifier>(*it).name);
        else if(std::holds_alternative<AstString>(*it))
            left.append(std::get<AstString>(*it).value);

        if(it != args.end() - 2)
            left.append(".");
    }

    const auto right = args.back();
    Assignment ass(Identifier(left), right);

    ast->statements.push_back(ass);
}

// Call

void make_call(Ast*& ast, const Instruction& instruction, const Function& function)
{
    // A represents the number of elements to keep on the stack. All popped elements
    // make up the name and the argument of the function.

    Collection<Expression> args;
    const auto             keep_stack_elements = A(instruction);
    while(ast->stack.size() > keep_stack_elements + 1)
    {
        args.push_back(std::get<Expression>(ast->stack.back()));
        ast->stack.pop_back();
    }

    auto caller = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();

    // Caller is a regular function
    if(std::holds_alternative<Identifier>(caller))
    {
        auto name = std::get<Identifier>(caller);

        std::reverse(args.begin(), args.end());

        if(B(instruction) == 0)
            ast->statements.push_back(Call(name, args));
        else
            ast->stack.push_back(Expression(Call(name, args)));
    }
    // Caller is a table
    else if(std::holds_alternative<AstTable>(caller))
    {
        auto table  = std::get<AstTable>(caller);
        auto map    = std::get<AstMap>(args[0]);
        table.pairs = map.pairs;
        ast->stack.push_back(table);
    }
    else
    {
        // TODO
        ast->stack.push_back(caller);
    }
}

void make_tail_call(Ast*& ast, const Instruction& instruction, const Function& function)
{
    // A represents the number of elements to keep on the stack. All popped elements
    // make up the name and the argument of the function.

    Collection<Expression> args;
    const auto             keep_stack_elements = A(instruction);
    while(ast->stack.size() > keep_stack_elements + 1)
    {
        args.push_back(std::get<Expression>(ast->stack.back()));
        ast->stack.pop_back();
    }

    auto name = std::get<Identifier>(std::get<Expression>(ast->stack.back()));
    ast->stack.pop_back();

    std::reverse(args.begin(), args.end());

    ast->statements.push_back(TailCall(name, args));
}

// For loops

void enter_for_loop(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    enter_block(ast);
}

void make_for_loop(Ast*& ast, const Instruction& instruction, const Function& function)
{
    exit_block(ast);

    // Locals are defined in a PC range. The first local defined from the current PC value is the
    // counter variable. Since this function handles the end of the for loop we have to subtract the
    // relative offset of the PC (S register of the instruction which is already negative).

    const auto counter = get_local_from_pc(function, PC + S(instruction), 0);

    const auto increment = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();

    const auto end = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();

    const auto begin = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();

    const ForLoop loop(counter, begin, end, increment, ast->child->statements);
    ast->statements.push_back(loop);
}

void make_for_in_loop(Ast*& ast, const Instruction& instruction, const Function& function)
{
    exit_block(ast);

    // Locals are defined in a PC range. The first local defined from the current PC value is the
    // list variable. Since this function handles the end of the for loop we have to subtract the
    // relative offset of the PC (S register of the instruction which is already negative). The
    // key variable is the next local defined after the list variable. The value variable is the
    // second next local defined after the list variable.

    const auto key   = get_local_from_pc(function, PC + S(instruction), 1);
    const auto value = get_local_from_pc(function, PC + S(instruction), 2);
    const auto right = std::get<Identifier>(std::get<Expression>(ast->stack.back())).name;
    ast->stack.pop_back();

    const ForInLoop loop(key, value, right, ast->child->statements);
    ast->statements.push_back(loop);
}

// While loop

void make_while_loop(Ast*& /*ast*/, const Instruction& /*instruction*/, const Function& /*function*/)
{
    // TODO: while loop does not really exist in byte code
}

// Condition

void make_condition(Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    const auto op = OP(instruction);

    if(op < Operator::JMPNE || op > Operator::JMPONF)
    {
        printf("Invalid binary operator OP %d (%s) \n", (int)op, OP_TO_STR[op].c_str());
        exit(1);
    }

    std::string comparison;

    switch(op)
    {
    case Operator::JMPNE:
        comparison = "==";
        break;
    case Operator::JMPEQ:
        comparison = "~=";
        break;
    case Operator::JMPLT:
        comparison = ">=";
        break;
    case Operator::JMPLE:
        comparison = ">";
        break;
    case Operator::JMPGT:
        comparison = "<=";
        break;
    case Operator::JMPGE:
        comparison = "<";
        break;
    case Operator::JMPT:
    case Operator::JMPONT:
        comparison = "~=";
        break;
    case Operator::JMPF:
    case Operator::JMPONF:
        comparison = "==";
        break;
    default:
        printf("OP %d not covered for conditions\n", (int)op);
        exit(1);
    }

    Collection<Expression> operands;

    if(op >= Operator::JMPNE && op <= Operator::JMPGE)
    {
        auto left = std::get<Expression>(ast->stack.back());
        ast->stack.pop_back();

        auto right = std::get<Expression>(ast->stack.back());
        ast->stack.pop_back();

        operands = {left, right};
    }
    else if(op >= Operator::JMPT && op <= Operator::JMPONF)
    {
        auto left = std::get<Expression>(ast->stack.back());
        ast->stack.pop_back();

        operands = {left, Identifier("nil")};
    }

    // if block
    if(!ast->context.is_condition)
    {
        const auto operation = AstOperation(comparison, operands);
        const auto block     = ConditionBlock(operation, Collection<Statement>{});
        const auto condition = Condition(Collection<ConditionBlock>{block});
        ast->statements.push_back(condition);
    }
    // elseif block
    else
    {
        // Collect the statements of the previous block and add them
        exit_block(ast);

        auto& condition                    = std::get<Condition>(ast->statements.back());
        condition.blocks.back().statements = ast->child->statements;

        const auto operation = AstOperation(comparison, operands);
        const auto block     = ConditionBlock(operation, Collection<Statement>{});
        condition.blocks.push_back(block);
    }

    // The following instructions make up the statements of the condition block
    enter_block(ast);

    ast->context.is_condition = true;
    ast->context.jump_offset  = PC + S(instruction);
}

void end_condition(Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    // TODO: Find the condition to identify the else condition (info must be somewhere in
    // the PC)

    if(ast->context.is_condition)
    {
        if(PC < ast->context.jump_offset)
        {
            exit_block(ast);

            auto&      condition = std::get<Condition>(ast->statements.back());
            const auto operation = AstOperation("", Collection<Expression>{});
            const auto block     = ConditionBlock(operation, Collection<Statement>{});
            condition.blocks.push_back(block);

            enter_block(ast);
        }
    }

    ast->context.is_condition = true;
    ast->context.jump_offset  = PC + S(instruction);
}

void make_closure(Ast*& ast, const Instruction& instruction, const Function& function)
{
    enter_block(ast);

    // TODO: Different handling of locals

    // Parse the closure body
    parse_function(ast, function.functions[A(instruction)]);

    // The locals are divided into arguments and local definitions.
    // The number of arguments depends on the leftover elements on the stack.
    // stack:             0 1  (2 local definitions)
    // arguments:   0 1 2      (3 closure arguments)
    // locals:      0 1 2 3 4

    const auto locals          = function.functions[A(instruction)].locals;
    const auto num_definitions = ast->stack.size();
    const auto num_arguments   = locals.size() - num_definitions;

    Collection<Identifier> arguments;
    for(size_t i = 0; i < num_arguments; ++i)
    {
        arguments.push_back(Identifier(locals[i].name));
    }

    Collection<LocalAssignment> local_definitions;
    for(size_t i = 0; i < num_definitions; ++i)
    {
        const auto local_index = num_arguments + num_definitions - i - 1;
        const auto local_name  = Identifier(locals[local_index].name);
        const auto local_value = std::get<Expression>(ast->stack.back());
        const auto ass         = LocalAssignment(local_name, local_value);
        local_definitions.push_back(ass);
        ast->stack.pop_back();
    }
    std::reverse(local_definitions.begin(), local_definitions.end());

    exit_block(ast);

    ast->stack.push_back(Closure(ast->child->statements, arguments, local_definitions));
}

// Public functions

void parse_function(Ast*& ast, const Function& function)
{
    for(const auto& i : function.instructions)
    {
        auto op = Operator(OP(i));

        if(TABLE.count(op) == 0)
        {
            printf("DEBUG: No action for %d (%s) in table\n", (int)op, OP_TO_STR[op].c_str());
            return;
        }

        TABLE[op](ast, i, function);

        // Handle conditions
        if(ast->context.is_condition)
        {
            if(PC == ast->context.jump_offset)
            {
                ast->context.is_condition = false;

                exit_block(ast);

                Condition& condition               = std::get<Condition>(ast->statements.back());
                condition.blocks.back().statements = ast->child->statements;
            }
        }

        PC++;
    }

    // TODO: Locals may be defined inline and have to be checked against the PC for every
    // instruction

    // Local definitions of the chunk
    if(ast->parent == nullptr && ast->stack.size())
    {
        Collection<Statement> statements;
        for(size_t i = 0; i < ast->stack.size(); ++i)
        {
            const auto local_name  = Identifier(function.locals[i].name);
            const auto local_value = std::get<Expression>(ast->stack[i]);
            const auto ass         = LocalAssignment(local_name, local_value);
            statements.push_back(ass);
        }
        ast->stack.clear();
        statements.insert(statements.end(), ast->statements.begin(), ast->statements.end());
        ast->statements = statements;
    }
}
