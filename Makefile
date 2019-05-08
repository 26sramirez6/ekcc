CC=g++
CFLAGS= -Wall -g3 -std=c++11 -Wno-reorder 

all: clean ekcc

test: clean ekcc
	@python3 ./tests/run-tests.py

valgrind: clean ekcc 
	valgrind --leak-check=full --track-origins=yes --log-file="valgrind.out" --show-reachable=yes -v ./ekcc

ekcc: Lexer.o Parser.o
	$(CC) $(CFLAGS) $^ -ll -o $@

Lexer.o: Lexer.cpp Parser.cpp AST.hpp CompilerConfig.hpp
	$(CC) $(CFLAGS) -c $< -o $@ -ll

Parser.o: Parser.cpp Lexer.cpp AST.hpp CompilerConfig.hpp
	$(CC) $(CFLAGS) -c $< -o $@ -ll

Lexer.cpp: Lexer.l Parser.y ValidTypes.hpp AST.hpp CompilerConfig.hpp
	flex --header-file=Lexer.hpp --outfile=$@ Lexer.l

Parser.cpp: Parser.y Lexer.cpp ValidTypes.hpp AST.hpp CompilerConfig.hpp
	bison -o $@ --defines=Parser.hpp Parser.y

Parser.hpp: Parser.cpp
	noop

clean:
	rm -f *.o *~ Lexer.cpp Lexer.hpp Parser.cpp Parser.hpp ekcc
	rm -r -f out
