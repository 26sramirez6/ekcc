CC=clang++
CFLAGS= -Wall -g3 -std=c++1z -Wno-sign-compare -Wno-unused-function -Wno-unused-command-line-argument -Wno-reorder -Wno-cast-qual
LLVM_CONFIG=--cxxflags --ldflags --system-libs --libs core executionengine mcjit interpreter analysis scalaropts vectorize native instcombine instrumentation ipo

# NOTE: SET THIS VARIABLE!!
LLVM_ROOT=/home/sramirez266/clang+llvm-7.0.0-x86_64-linux-gnu-ubuntu-16.04
COMPILE_CMD=$(LLVM_ROOT)/bin/$(CC)
LLVM_CONFIG_CMD=`$(LLVM_ROOT)/bin/llvm-config $(LLVM_CONFIG)`

all: clean main.ll ekcc

test: clean main.ll ekcc
	@python3 ./tests/run-tests.py

valgrind: clean ekcc 
	valgrind --leak-check=full --track-origins=yes --log-file="valgrind.out" --show-reachable=yes -v ./ekcc

ekcc: Lexer.o Parser.o
	$(COMPILE_CMD) $(CFLAGS) $^ $(LLVM_CONFIG_CMD) -ll -D CLANG_BINARY="$(COMPILE_CMD)" -o $@
	
Lexer.o: Lexer.cpp Parser.cpp AST.hpp CompilerConfig.hpp LLVMGlobals.hpp
	$(COMPILE_CMD) $(CFLAGS) -c $< $(LLVM_CONFIG_CMD) -D CLANG_BINARY="$(COMPILE_CMD)" -o $@

Parser.o: Parser.cpp Lexer.cpp AST.hpp CompilerConfig.hpp LLVMGlobals.hpp
	$(COMPILE_CMD) $(CFLAGS) -c $< $(LLVM_CONFIG_CMD) -D CLANG_BINARY=$(COMPILE_CMD) -o $@

Lexer.cpp: Lexer.l Parser.y ValidTypes.hpp AST.hpp CompilerConfig.hpp LLVMGlobals.hpp
	flex --header-file=Lexer.hpp --outfile=$@ Lexer.l

Parser.cpp: Parser.y Lexer.cpp ValidTypes.hpp AST.hpp CompilerConfig.hpp LLVMGlobals.hpp
	bison -o $@ --defines=Parser.hpp Parser.y

Parser.hpp: Parser.cpp
	noop

main.ll: main.c
	$(LLVM_ROOT)/bin/clang -O0 -S -emit-llvm $< -o $@ 

clean:
	rm -f *.o *~ Lexer.cpp Lexer.hpp Parser.cpp Parser.hpp main.ll ekcc
	rm -r -f out
