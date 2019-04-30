%{
    #include <stdio.h>
    #include <cstdio>
    #include <iostream>
    #include "Expression.h"
    #include "ValidTypes.hpp"
    #include "AST.hpp"
    #include "Lexer.hpp"
    #include "Parser.hpp"
    #include "AST.hpp"
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

  Variable variable;
  ASTNode * node;
  SExpression *expression;
}

// declare literals
%token <ival> T_INT_LITERAL
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
%token <noliasType> T_TYPE_NOALIAS

%token <ifControl> T_CONTROL_IF
%token <elseControl> T_CONTROL_ELSE
%token <whileControl> T_CONTROL_WHILE
%token <forControl> T_CONTROL_FOR
%token <returnControl> T_CONTROL_RETURN

%token <printFunction> T_FUNCTION_PRINT
%token <runFunction> T_FUNCTION_RUN
%token <defFunction> T_FUNCTION_DEF
%token <externFunction> T_FUNCTION_EXTERN 

%token <sval> T_IDENT
%token <sval> T_VARID

// declare symbols
%token T_LBRACE "{"
%token T_RBRACE "}"
%token T_LPAREN "("
%token T_RPAREN ")"
%token T_SEMICOLON ";"
%token T_LBRACKET "["
%token T_RBRACKET "]"
%token T_DOLLAR "$"
%token T_POUND "#"
%token T_COMMA ","
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
%token T_ASSIGN "="

// declare types
%type <node> funcs
%type <node> externs
%type <node> vdecl

%parse-param { ASTNode **root }

%%

prog:
  funcs { root->Build($1); }
  | externs funcs { root->Build($1, $2); }
  ;

externs:
  extern { cout << "name: externs" << endl; }
  | extern externs { cout << "name: externs" << endl; } //push_back extern
  ;

extern:
  T_FUNCTION_EXTERN type globid "(" ")" ";" { cout << "name: extern" << endl; }
  | T_FUNCTION_EXTERN type globid "(" tdecls ")" ";" { cout << "name: extern" << endl; }
  ;

funcs:
  func {}
  | func funcs { cout << "name: funcs" << endl; } //push_back funcs
  ;

func:
  T_FUNCTION_DEF type globid "(" ")" blk { cout << "name: func" << endl; }
  | T_FUNCTION_DEF type globid "(" vdecls ")" blk { cout << "name: func" << endl; }
  | T_FUNCTION_DEF type T_FUNCTION_RUN "("")" blk { cout << "name: func" << endl; }
  ;

blk:
  "{" "}" { cout << "name: bloc" << endl; }
  | "{" stmts "}" { cout << "name: bloc" << endl; }
  ; 

stmts:
  stmt {}
  | stmt stmts { cout << "name: stmts" << endl; }
  ;

stmt:
  blk { cout << "name: stmt" << endl; }
  | T_CONTROL_RETURN ";" { cout << "name: return_stmt" << endl; }
  | T_CONTROL_RETURN exp ";" { cout << "name: return_stmt" << endl; }
  | vdecl "=" exp ";" { cout << "name: assign_stmt" << endl; }
  | exp ";" { cout << "name: stmt" << endl; }
  | T_CONTROL_WHILE "(" exp ")" stmt { cout << "name: while_stmt" << endl; }
  | T_CONTROL_IF "(" exp ")" stmt { cout << "name: if_stmt" << endl; }
  | T_CONTROL_IF "(" exp ")" stmt  T_CONTROL_ELSE stmt { cout << "name: if_else_stmt" << endl; }
  | T_FUNCTION_PRINT exp ";" { cout << "name: print_stmt" << endl; }
  | T_FUNCTION_PRINT slit ";" { cout << "name: print_stmt" << endl; }
  ;

exp:
  "(" exp ")" { cout << "exp" << endl; }
  | binop { cout << "exp" << endl; }
  | uop { cout << "exp" << endl; }
  | lit { cout << "exp" << endl; }
  | varid { cout << "exp" << endl; }
  | globid "(" ")" { cout << "exp" << endl; }
  | globid "(" exp ")" { cout << "exp" << endl; }
  ;

binop:
  arithops { cout << "binop" << endl; }
  | logicops { cout << "binop" << endl; }
  | varid "=" exp { cout << "binop" << endl; }
  | "[" type "]" exp { cout << "binop" << endl; }
  ;

arithops:
  exp "*" exp { cout << "arithops" << endl; }
  | exp "/" exp { cout << "arithops" << endl; }
  | exp "+" exp { cout << "arithops" << endl; }
  | exp "-" exp { cout << "arithops" << endl; }
  ;

logicops:
  exp "==" exp { cout << "logicops" << endl; }
  | exp "<" exp { cout << "logicops" << endl; }
  | exp ">" exp { cout << "logicops" << endl; }
  | exp "&&" exp { cout << "logicops" << endl; }
  | exp "||" exp { cout << "logicops" << endl; }
  ;

uop:
  "!" exp { cout << "uop" << endl; }
  | "-" exp { cout << "uop" << endl; }
  ;

vdecls:
  vdecl {}
  | vdecl "," vdecls { cout << "vdecls" << endl; }
  ;

tdecls:
  type
  | type "," tdecls { cout << "tdecls" << endl; }
  ;

vdecl:
  type T_VARID { cout << "vedcl: " << $2 << endl; }
  ;

type:
  T_TYPE_INT { cout << "int_type" << endl; }
  | T_TYPE_CINT { cout << "cint_type" << endl; }
  | T_TYPE_FLOAT { cout << "float_type" << endl; }
  | T_TYPE_BOOL { cout << "bool_type" << endl; }
  | T_TYPE_VOID { cout << "void_type" << endl; }
  | T_TYPE_REF type { cout << "ref_type" << endl; }
  | T_TYPE_NOALIAS T_TYPE_REF type { cout << "noalas_ref" << endl; }
  ;

varid:
  T_VARID { cout << "varid: " << $1 << endl;}
  ;

globid:
  T_IDENT { cout << "indentifier: " << $1 << endl; }
  ;

lit:
  T_INT_LITERAL { cout << "int: " << $1 << endl; }
  | T_FLOAT_LITERAL { cout << "float: " << $1 << endl; }
  | T_BOOL_FALSE_LITERAL { cout << "bool: false" << endl; }
  | T_BOOL_TRUE_LITERAL { cout << "bool: true" << endl; }
  ;


slit:
  T_STRING_LITERAL { cout << "string: " << $1 << endl; }
  ;

%%

int main(int, char**) {
  // open a file handle to a particular file:
  FILE *myfile = fopen("test_simple.ek", "r");
  // make sure it's valid:
  if (!myfile) {
    cout << "I can't open file!" << endl;
    return -1;
  }
  // Set flex to read from it instead of defaulting to STDIN:
  yyin = myfile;

  // Set up the root tree
  ProgramNode * root = new ProgramNode();
  
  // Parse through the input:
  yyparse(&root);
  
  // Print the AST Tree
  root.Print();
}

void yyerror(const char *s) {
  cout << "EEK, parse error!  Message: " << s << endl;
  // might as well halt now:
  exit(-1);
}