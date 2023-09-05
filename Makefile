CC = g++
BIN = lua4dec
BUILDDIR = make
INC = -I source/
SRC = \
    source/lua4dec.cpp \
    source/ast/ast.cpp \
    source/lua/lua.cpp \
    source/parser/parser.cpp
OBJ = $(SRC:%.c=$(BUILDDIR)/%.o)
CFLAGS = -Wall -Wextra -ansi -pedantic -std=c++17 -g


test:
	echo $(OBJ)

all: clean $(BIN)

clean:
	rmdir $(BUILDDIR)
	mkdir $(BUILDDIR)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(INC) -o $(BIN) $(OBJ) $(LDFLAGS)

$(BUILDDIR)$(PATHSEP)%.o: %.c
	mkdir $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $<