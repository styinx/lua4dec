#include "parser/parser.hpp"

unsigned PC           = 0;
bool     is_condition = false;

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
    const auto name   = function.locals[U(instruction)];
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
    const auto name = function.locals[U(instruction)];
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
    // Its only a table if an identifier is on the stack before. Otherwise its a map or list.

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
void add(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto right = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    const auto left = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    ast->stack.push_back(AstOperation("+", {left, right}));
}

void addi(Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    const auto right = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    const auto left = AstNumber(S(instruction));
    ast->stack.push_back(AstOperation("+", {left, right}));
}

void sub(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto right = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    const auto left = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    ast->stack.push_back(AstOperation("-", {left, right}));
}

void mult(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto right = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    const auto left = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    ast->stack.push_back(AstOperation("*", {left, right}));
}

void div(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto right = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    const auto left = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    ast->stack.push_back(AstOperation("/", {left, right}));
}

void pow(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto right = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    const auto left = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    ast->stack.push_back(AstOperation("^", {left, right}));
}

void concat(Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    Collection<Expression> ex;
    for(unsigned i = 0; i < U(instruction); ++i)
    {
        ex.push_back(std::get<Expression>(ast->stack.back()));
        ast->stack.pop_back();
    }
    ast->stack.push_back(AstOperation("..", ex));
}

void minus(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    const auto right = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();
    ast->stack.push_back(AstOperation("-", {right}));
}

void not(Ast * &ast, const Instruction& /*instruction*/, const Function& /*function*/)
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
    const auto left  = Identifier(function.locals[B(instruction)]);
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

// For loop

void make_for_loop(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    exit_block(ast);

    const auto increment = std::get<AstInt>(std::get<Expression>(ast->stack.back()));
    ast->stack.pop_back();

    const auto end = std::get<AstInt>(std::get<Expression>(ast->stack.back()));
    ast->stack.pop_back();

    const auto begin = std::get<AstInt>(std::get<Expression>(ast->stack.back()));
    ast->stack.pop_back();

    const ForLoop loop(begin, end, increment, ast->child->statements);
    ast->statements.push_back(loop);
}

// For in loop

void make_for_in_loop(Ast*& ast, const Instruction& /*instruction*/, const Function& /*function*/)
{
    exit_block(ast);

    ForInLoop loop;

    // loop.right = std::get<Identifier>(std::get<Expression>(ast->stack.back())).name;
    ast->stack.pop_back();

    loop.statements = ast->child->statements;

    ast->statements.push_back(loop);
}

// While loop

void make_while_loop(Ast*& /*ast*/, const Instruction& /*instruction*/, const Function& /*function*/)
{
    // TODO: while loop does not really exist in byte code
}

// Condition

void make_binary_condition(Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    const auto op = OP(instruction);

    if(op < Operator::JMPNE || op > Operator::JMPGE)
    {
        printf("Invalid binary operator OP %d (%s) \n", (int)op, OP_TO_STR[op].c_str());
    }

    auto left = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();

    auto right = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();

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
    default:
        printf("OP %d not covered for conditions\n", (int)op);
    }

    const auto operation = AstOperation(comparison, {left, right});
    const auto condition = Condition(operation, Collection<Statement>{});
    ast->statements.push_back(condition);

    enter_block(ast);

    is_condition             = true;
    PC                       = S(instruction);
    ast->context.condition   = true;
    ast->context.jump_offset = S(instruction);
}

void make_unary_condition(Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    const auto op = OP(instruction);

    if(op < Operator::JMPT || op > Operator::JMPONF)
    {
        printf("Invalid unary operator OP %d (%s) \n", (int)op, OP_TO_STR[op].c_str());
    }

    std::string comparison;

    switch(op)
    {
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
    }

    const auto left = std::get<Expression>(ast->stack.back());
    ast->stack.pop_back();

    const auto operation = AstOperation(comparison, {left, Identifier("nil")});
    const auto condition = Condition(operation, Collection<Statement>{});
    ast->statements.push_back(condition);

    enter_block(ast);

    is_condition             = true;
    PC                       = S(instruction);
    ast->context.condition   = true;
    ast->context.jump_offset = S(instruction);
}

void end_condition(Ast*& ast, const Instruction& instruction, const Function& /*function*/)
{
    const auto operation = AstOperation("else/elseif", {Identifier(""), Identifier("")});
    const auto condition = Condition(operation, Collection<Statement>{});
    ast->statements.push_back(condition);

    enter_block(ast);

    is_condition             = true;
    PC                       = S(instruction);
    ast->context.condition   = true;
    ast->context.jump_offset = S(instruction);
}

void make_closure(Ast*& ast, const Instruction& instruction, const Function& function)
{
    enter_block(ast);

    // TODO: pushes dummies on stack. local definitions can not be read anymore
    const auto locals = function.functions[A(instruction)].locals;
    for(const auto& local : locals)
    {
        ast->stack.push_back(Identifier("TODO"));
    }

    parse_function(ast, function.functions[A(instruction)]);

    // Leftover elements on the stack make up the local variables.
    unsigned                    i = 0;
    Collection<LocalAssignment> args;
    for(const auto& local : locals)
    {
        ast->stack.pop_back();
    }
    for(const auto& e : ast->stack)
    {
        const auto ass = LocalAssignment(Identifier(locals[i++]), std::get<Expression>(e));
        args.push_back(ass);
    }

    exit_block(ast);

    ast->stack.push_back(Closure(ast->child->statements, args));
}

// Public functions

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
    {Operator::ADD,         &add},
    {Operator::ADDI,        &addi},
    {Operator::SUB,         &sub},
    {Operator::MULT,        &mult},
    {Operator::DIV,         &div},
    {Operator::POW,         &pow},
    {Operator::CONCAT,      &concat},
    {Operator::MINUS,       &minus},
    {Operator::NOT,         &not},
    // Call
    {Operator::CALL,        &make_call},
    {Operator::TAILCALL,    &make_tail_call},
    // Assignment
    {Operator::SETLOCAL,    &make_local_assignment},
    {Operator::SETGLOBAL,   &make_assignment},
    {Operator::SETTABLE,    &make_table_assignment},
    // For loop
    {Operator::FORPREP,     &enter_block},
    {Operator::FORLOOP,     &make_for_loop},
    // For in loop
    {Operator::LFORPREP,    &enter_block},
    {Operator::LFORLOOP,    &make_for_in_loop},
    // Conditions
    {Operator::JMPNE,       &make_binary_condition},
    {Operator::JMPEQ,       &make_binary_condition},
    {Operator::JMPLT,       &make_binary_condition},
    {Operator::JMPLE,       &make_binary_condition},
    {Operator::JMPGT,       &make_binary_condition},
    {Operator::JMPGE,       &make_binary_condition},
    {Operator::JMPT,        &make_unary_condition},
    {Operator::JMPF,        &make_unary_condition},
    {Operator::JMPONT,      &make_unary_condition},
    {Operator::JMPONF,      &make_unary_condition},
    {Operator::JMP,         &end_condition},
    // Closure
    {Operator::CLOSURE,     &make_closure},
};

// clang-format on

void parse_function(Ast*& ast, const Function& function)
{
    // TODO: Add local variables for this scope

    for(const auto& i : function.instructions)
    {
        // We are currently inside a condition
        // if(ast->context.condition)
        if(is_condition)
            // if(ast->context.jump_offset > 0)
            if(PC > 0)
                // ast->context.jump_offset--;
                PC--;

        auto op = Operator(OP(i));

        if(TABLE.count(op) == 0)
        {
            printf("DEBUG: No action for %d (%s) in table\n", (int)op, OP_TO_STR[op].c_str());
            return;
        }

        TABLE[op](ast, i, function);

        // Last instruction inside the condition
        // if(ast->context.condition)
        if(is_condition)
        {
            // if(ast->context.jump_offset == 0)
            if(PC == 0)
            {
                // ast->context.condition = false;
                is_condition = false;

                exit_block(ast);

                Condition& condition = std::get<Condition>(ast->statements.back());
                condition.statements = ast->child->statements;
            }
        }
    }
}
