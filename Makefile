CC=clang++
CFLAGS= -Wall -g3 -std=c++11 -Wno-reorder -Wno-sign-compare -Wno-unused-function
LLVM_CONFIG=--cxxflags --ldflags --system-libs --libs core

# NOTE: SET THIS VARIABLE!!
LLVM_ROOT=/home/sramirez266/clang+llvm-7.0.0-x86_64-linux-gnu-ubuntu-16.04
COMPILE_CMD="$(LLVM_ROOT)/bin/$(CC)"
LLVM_CONFIG_CMD=`"$(LLVM_ROOT)/bin/llvm-config" $(LLVM_CONFIG)`

all: clean ekcc

test: clean ekcc
	@python3 ./tests/run-tests.py

valgrind: clean ekcc 
	valgrind --leak-check=full --track-origins=yes --log-file="valgrind.out" --show-reachable=yes -v ./ekcc

ekcc: Lexer.o Parser.o
	$(COMPILE_CMD) $(CFLAGS) $(CONFIG_CMD) $^ -ll -o $@

Lexer.o: Lexer.cpp Parser.cpp AST.hpp CompilerConfig.hpp
	$(COMPILE_CMD) $(CFLAGS) $(CONFIG_CMD) -c $< -o $@ -ll

Parser.o: Parser.cpp Lexer.cpp AST.hpp CompilerConfig.hpp
	$(COMPILE_CMD) $(CFLAGS) $(CONFIG_CMD) -c $< -o $@ -ll

Lexer.cpp: Lexer.l Parser.y ValidTypes.hpp AST.hpp CompilerConfig.hpp
	flex --header-file=Lexer.hpp --outfile=$@ Lexer.l

Parser.cpp: Parser.y Lexer.cpp ValidTypes.hpp AST.hpp CompilerConfig.hpp
	bison -o $@ --defines=Parser.hpp Parser.y

Parser.hpp: Parser.cpp
	noop
	
clean:
	rm -f *.o *~ Lexer.cpp Lexer.hpp Parser.cpp Parser.hpp ekcc
	rm -r -f out
