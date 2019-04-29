CC=g++
CFLAGS= -Wall -Werror -Wextra -g -std=c++11 -Wno-deprecated-register -Wno-unused-parameter -Wno-unneeded-internal-declaration -Wno-unused-function

all: clean ekcc

ekcc: ekcc.cpp Lexer.cpp Parser.cpp ValidTypes.cpp Expression.cpp
	$(CC) $(CFLAGS) $^ -o $@
	cat test.ek | ./$@

Lexer.o: Lexer.cpp Parser.cpp
	$(CC) $(CFLAGS) -c $< -o $@

Parser.o: Parser.cpp Lexer.cpp
	$(CC) $(CFLAGS) -c $< -o $@

Lexer.cpp: Lexer.l Parser.y
	flex --header-file=Lexer.hpp --outfile=$@ Lexer.l

Parser.cpp: Parser.y Lexer.cpp ValidTypes.hpp
	bison -o $@ --defines=Parser.hpp Parser.y

Parser.hpp: Parser.cpp
	noop

clean:
	rm -f *.o *~ Lexer.cpp Lexer.hpp Parser.cpp Parser.hpp ekcc