%{
    #include <stdio.h>
    #include <cstdio>
    #include <iostream>
    #include "Expression.h"
    #include "ValidTypes.hpp"
    #include "Lexer.hpp"
    #include "Parser.hpp"
    using namespace std;
    // stuff from flex that bison needs to know about:
    extern int yylex();
    extern int yyparse();
    extern FILE *yyin;
  
    void yyerror(const char *s);

%}

%code requires {
  #include "ValidTypes.hpp"
  #include "Expression.h"
}

%union{
  int ival;
  bool bval;
  float fval;
  char *sval;
  IntType intType;
  FloatType floatType;
  CintType cintType;
  BoolType boolType;
  VoidType voidType;
  RefType refType;

  IfControl ifControl;
  ElseControl elseControl;
  WhileControl whileControl;
  ForControl forControl;
  ReturnControl returnControl;

  PrintFunction printFunction;
  RunFunction runFunction;
  DefFunction defFunction;
  ExternFunction externFunction;

  SExpression *expression;
}

// declare literals
%token <ival> T_INT_LITERAL "number"
%token <fval> T_FLOAT_LITERAL
%token <sval> T_STRING_LITERAL
%token <bval> T_BOOL_FALSE_LITERAL
%token <bval> T_BOOL_TRUE_LITERAL

// declare types
%token <intType> T_TYPE_INT
%token <floatType> T_TYPE_FLOAT
%token <cintType> T_TYPE_CINT
%token <boolType> T_TYPE_BOOL
%token <voidType> T_TYPE_VOID
%token <refType> T_TYPE_REF

%token <ifControl> T_CONTROL_IF
%token <elseControl> T_CONTROL_ELSE
%token <whileControl> T_CONTROL_WHILE
%token <forControl> T_CONTROL_FOR
%token <returnControl> T_CONTROL_RETURN

%token <printFunction> T_FUNCTION_PRINT
%token <runFunction> T_FUNCTION_RUN
%token <defFunction> T_FUNCTION_DEF
%token <externFunction> T_FUNCTION_EXTERN 

// declare symbols
%token T_LPAREN "("
%token T_RPAREN ")"
%token T_SEMICOLON ";"
%token T_LBRACKET "["
%token T_RBRACKET "]"
%token T_DOLLAR "$"
%token T_POUND "#"
// declare arithmetics
%token T_PLUS "+"
%token T_STAR "*"
%token T_FSLASH "/"
%token T_HYPHEN "-"
// declare comparators
%token T_GT ">"
%token T_LT "<"
%token T_OR "||"
%token T_AND "&&"
%token T_EQ "=="
// declare unary
%token T_NEG "!"

// %type <expression> expr

%left "+"
%left "*"

%%

end:
a a { cout << "the end of file" << endl;}
;


a:
  T_TYPE_INT { cout << "parser_type: int" << endl; }
  | T_CONTROL_IF { cout << "parser_control: if" << endl;}
  ;


%%

int main(int, char**) {
  // open a file handle to a particular file:
  FILE *myfile = fopen("test.ek", "r");
  // make sure it's valid:
  if (!myfile) {
    cout << "I can't open a.snazzle.file!" << endl;
    return -1;
  }
  // Set flex to read from it instead of defaulting to STDIN:
  yyin = myfile;

  // Parse through the input:
  yyparse();
}

void yyerror(const char *s) {
  cout << "EEK, parse error!  Message: " << s << endl;
  // might as well halt now:
  exit(-1);
}