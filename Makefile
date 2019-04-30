CC=g++
CFLAGS= -Wall -g -std=c++11 #-Werror -Wextra -Wno-deprecated-register -Wno-unused-parameter -Wno-unneeded-internal-declaration -Wno-unused-function -Wno-sign-compare 

all: clean ekcc

ekcc: Lexer.o Parser.o Expression.o
	$(CC) $(CFLAGS) $^ -lfl -o $@

Expression.o: Expression.cpp
	$(CC) $(CFLAGS) -c $< -o $@

ValidTypes.o: ValidTypes.cpp
	$(CC) $(CFLAGS) -c $< -o $@

Lexer.o: Lexer.cpp Parser.cpp AST.hpp
	$(CC) $(CFLAGS) -c $< -o $@ -lfl

Parser.o: Parser.cpp Lexer.cpp AST.hpp
	$(CC) $(CFLAGS) -c $< -o $@ -lfl

Lexer.cpp: Lexer.l Parser.y
	flex --header-file=Lexer.hpp --outfile=$@ Lexer.l

Parser.cpp: Parser.y Lexer.cpp ValidTypes.hpp
	bison -o $@ --defines=Parser.hpp Parser.y

Parser.hpp: Parser.cpp
	noop

clean:
	rm -f *.o *~ Lexer.cpp Lexer.hpp Parser.cpp Parser.hpp ekcc