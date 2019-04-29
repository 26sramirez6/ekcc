%{
    #include <stdio.h>
    #include <cstdio>
    #include <iostream>
    using namespace std;
    // #include "ast.h"
    extern int yylex();
    extern int yyparse();
    extern FILE *yyin;
    void yyerror(const char * s);
%}

%defines "Parser.h"

%union {
    int ival;
    float fval;
    char *sval;
}

// declare type tokens
%token <ival> T_INT
%token <fval> T_FLOAT
%token <sval> T_STRING

%%

test:
    T_INT {cout << "num: " << $1 << endl;}
    | T_STRING {cout << "string: " << $1 << endl;}
    ;

%%

void yyerror(const char *s) {
  cout << "Message: " << s << endl;
  // might as well halt now:
//   exit(-1);
}

int main(int, char**) {
  // open a file:
//   int fd = open("test.ek", O_RDONLY);
//   int pagesize = getpagesize();
//   char *data = mmap(0, pagesize, PROT_READ, MAP_SHARED, fd, pagesize);
//   yyin = data;
    FILE *input_file = fopen("test.ek", "r");
    yyin = input_file;
    yyparse();
}


