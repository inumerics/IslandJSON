# ==============================================================================
# Toolchain Configuration
#
CC      := gcc
CFLAGS  := -Iinclude

LEXER   := island-lexer
PARSER 	:= island-parser
BIN   	:= example

# ==============================================================================
# Grammar Inputs / Generated Outputs
#
SCANNER_L := parser/scanner.l
GRAMMAR_Y := parser/grammar.y

LEX_C := lex.yy.c
LEX_O := lex.yy.o
TAB_H := grammar.tab.h
TAB_C := grammar.tab.c
TAB_O := grammar.tab.o

# ==============================================================================
# Sources and Objects
#
HDRS := $(wildcard include/*.h src/*.h)
SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:.c=.o) $(LEX_O) $(TAB_O)

# ==============================================================================
# Build Rules
#
.PHONY: all

all: $(BIN)

$(TAB_C) $(TAB_H): $(GRAMMAR_Y)
	$(PARSER) -d $<
	
$(LEX_C): $(SCANNER_L) $(TAB_H)
	$(LEXER) -o $@ $<
	
$(TAB_O): $(TAB_C)
$(LEX_O): $(LEX_C)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJS) $(HDRS)
	$(CC) -o $(BIN) $(CFLAGS) $(OBJS)
	
# ==============================================================================
# Utility Targets
#
.PHONY: rebuild clean clean-objs test
	
rebuild: clean
	$(MAKE) $(BIN)

clean-objs:
	@rm -f $(LEX_C) $(TAB_C) $(TAB_H)
	@rm -f $(OBJS)

clean: clean-objs
	@rm -f $(BIN)

test: $(BIN)
	./$(BIN) tests/input.json

