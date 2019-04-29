%{
    #include <stdio.h>
    #include <cstdio>
    #include <iostream>
    #include "Expression.h"
    #include "ValidTypes.hpp"
    #include "Lexer.hpp"
    #include "Parser.hpp"
    using namespace std;
    // #include "ast.h"
    // extern int yylex();
    //extern int yyparse();
    // extern FILE * yyin;
    
    int yyerror(SExpression **expression, yyscan_t scanner, const char *msg) {
    /* Add error handling routine as needed */
    }

%}

%code requires {
  #include "ValidTypes.hpp"
  #include "Expression.h"
  typedef void* yyscan_t;
}

%define api.pure
%lex-param   { yyscan_t scanner }
%parse-param { SExpression **expression }
%parse-param { yyscan_t scanner }

%union {
  int ival;
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
%token T_PLUS "+"
%token T_STAR "*"



%type <expression> expr

%left "+"
%left "*"

%%

input
    : expr { *expression = $1; }
    ;

expr
    : expr[L] "+" expr[R] { $$ = createOperation( eADD, $L, $R ); }
    | expr[L] "*" expr[R] { $$ = createOperation( eMULTIPLY, $L, $R ); }
    | "(" expr[E] ")"     { $$ = $E; }
    | "number"            { $$ = createNumber($1); }
    ;

%%