FILES=Lexer.c Parser.c Expression.c main.c
CC=g++
CFLAGS= -Wall -Werror -Wextra -g -std=c++1z

test: $(FILES) 
	$(CC) $(CFLAGS) $(FILES) -o test

Lexer.c: Lexer.l Parser.y
	flex -o $@ Lexer.l

test_lexer: Lexer.c
	$(CC) $(CFLAGS) $< -o $@ -ll
	cat test.ek | ./$@

Parser.c: Parser.y Lexer.c
	bison -o $@ Parser.y

Parser.h: Parser.c
	noop

clean:
	rm -f *.o *~ Lexer.c Lexer.h Parser.c Parser.h test
