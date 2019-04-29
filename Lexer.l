%{
  #include "Parser.h"
  #include <iostream>
  using namespace std;
%}
%option noyywrap
%%
[ \t\n]         ;
\"[^\n"]+\"     { cout << "String" << yytext << endl; return T_STRING; }
[0-9]+          { cout << "INT" << yytext << endl; return T_INT; }
[0-9]+\.[0-9]+  { cout << "FLOAT" << yytext << endl; return T_FLOAT; }
%%
int main(int, char**) {
  // open a file:
    FILE *input_file = fopen("test.ek", "r");
    yyin = input_file;
    while(yylex());
}

