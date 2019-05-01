CC=g++
CFLAGS= -Wall -g3 -std=c++11 #-Werror -Wextra -Wno-deprecated-register -Wno-unused-parameter -Wno-unneeded-internal-declaration -Wno-unused-function -Wno-sign-compare 

all: clean ekcc

valgrind: clean ekcc 
	valgrind --leak-check=full --track-origins=yes --log-file="valgrind.out" --show-reachable=yes -v ./ekcc

ekcc: Lexer.o Parser.o
	$(CC) $(CFLAGS) $^ -lfl -o $@
	./ekcc > ekcc.yaml

Lexer.o: Lexer.cpp Parser.cpp AST.hpp
	$(CC) $(CFLAGS) -c $< -o $@ -lfl

Parser.o: Parser.cpp Lexer.cpp AST.hpp
	$(CC) $(CFLAGS) -c $< -o $@ -lfl

Lexer.cpp: Lexer.l Parser.y ValidTypes.hpp AST.hpp
	flex --header-file=Lexer.hpp --outfile=$@ Lexer.l

Parser.cpp: Parser.y Lexer.cpp ValidTypes.hpp AST.hpp
	bison -o $@ --defines=Parser.hpp Parser.y

Parser.hpp: Parser.cpp
	noop

clean:
	rm -f *.o *~ Lexer.cpp Lexer.hpp Parser.cpp Parser.hpp ekcc