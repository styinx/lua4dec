CC = g++
BIN = luadec
LIB = lua4dec.a
BUILDDIR = make
INC = -I source/
SRC_LIB = \
    source/lua4dec.cpp \
    source/ast/ast.cpp \
    source/lua/lua.cpp \
    source/parser/parser.cpp
SRC_BIN = $(SRC_LIB) source/main.cpp
OBJ_LIB = $(SRC_LIB:%.c=$(BUILDDIR)/%.o)
OBJ_BIN = $(SRC_BIN:%.c=$(BUILDDIR)/%.o)
CFLAGS = -Wall -Wextra -ansi -pedantic -std=c++17 -g
LDFLAGS = lua4dec


all: clean $(LIB) $(BIN)

clean:
	rmdir $(BUILDDIR)
	mkdir $(BUILDDIR)

$(LIB): $(OBJ_LIB)
	ar rcs $(LIB) $(OBJ_LIB)

$(BIN): $(OBJ_BIN)
	$(CC) $(CFLAGS) $(INC) -o $(BIN) $(OBJ_BIN) $(LDFLAGS)

$(BUILDDIR)$(PATHSEP)%.o: %.c
	mkdir $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $<
