%{
    #include <stdio.h>
    #include <cstdio>
    #include <iostream>
    #include "Expression.h"
    #include "ValidTypes.hpp"
    #include "AST.hpp"
    #include "Lexer.hpp"
    #include "Parser.hpp"
    using namespace std;
    // stuff from flex that bison needs to know about:
    extern int yylex();
    extern int yyparse();
    extern FILE *yyin;
  
    void yyerror(ProgramNode ** node, const char *s) {
    	
    }

%}

%code requires {
  #include "ValidTypes.hpp"
  #include "AST.hpp"
  #include "Expression.h"
}

%union{
  int ival;
  bool bval;
  float fval;
  char *sval;
//  IntType intType;
//  FloatType floatType;
//  CintType cintType;
//  BoolType boolType;
//  VoidType voidType;
//  RefType refType;

  IfControl ifControl;
  ElseControl elseControl;
  WhileControl whileControl;
  ForControl forControl;
  ReturnControl returnControl;

  PrintFunction printFunction;
  RunFunction runFunction;
  DefFunction defFunction;
  ExternFunction externFunction;
  
  ValidType * validType;
  ASTNode * node;
  ProgramNode * programNode;
  FuncNode * funcNode;
  FuncsNode * funcsNode;
  ExternNode * externNode;
  ExternsNode * externsNode;
  BlockNode * blockNode;
  VdeclNode * vdeclNode;
  VdeclsNode * vdeclsNode;
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
%type <validType> type
%type <programNode> prog
%type <funcsNode> funcs
%type <externsNode> externs
%type <vdeclNode> vdecl
%type <vdeclsNode> vdecls
%type <funcNode> func
%type <externNode> extern
%type <blockNode> blk
%type <sval> globid

%parse-param { ProgramNode ** root }

%%

prog:
  funcs { $$ = new ProgramNode($1); (*root) = $$; }
  | externs funcs { $$ = new ProgramNode($1, $2); (*root) = $$; }
  ;

externs:
  extern { }
  | extern externs { }
  ;

extern:
  T_FUNCTION_EXTERN type globid "(" ")" ";" { $$ = new ExternNode(); }
  | T_FUNCTION_EXTERN type globid "(" tdecls ")" ";" { $$ = new ExternNode(); }
  ;

funcs:
  func { $$ = new FuncsNode($1); }
  | funcs func { $$ = new FuncsNode($1, $2); }
  ;

func:
  T_FUNCTION_DEF type globid "(" ")" blk { $$ = new FuncNode($2, $3, $6); }
  | T_FUNCTION_DEF type globid "(" vdecls ")" blk { $$ = new FuncNode($2, $3, $5, $7); }
  | T_FUNCTION_DEF type T_FUNCTION_RUN "(" ")" blk { $$ = new FuncNode($2, "run", $6); }
  ;

blk:
  "{" "}" { $$ = new BlockNode(); }
  | "{" stmts "}" { $$ = new BlockNode(); }
  ; 

stmts:
  stmt {}
  | stmt stmts { }
  ;

stmt:
  blk { }
  | T_CONTROL_RETURN ";" { }
  | T_CONTROL_RETURN exp ";" { }
  | vdecl "=" exp ";" { }
  | exp ";" { }
  | T_CONTROL_WHILE "(" exp ")" stmt { }
  | T_CONTROL_IF "(" exp ")" stmt { }
  | T_CONTROL_IF "(" exp ")" stmt  T_CONTROL_ELSE stmt { }
  | T_FUNCTION_PRINT exp ";" { }
  | T_FUNCTION_PRINT slit ";" { }
  ;

exp:
  "(" exp ")" { }
  | binop { }
  | uop {  }
  | lit {  }
  | varid {  }
  | globid "(" ")" {  }
  | globid "(" exp ")" {  }
  ;

binop:
  arithops {  }
  | logicops {  }
  | varid "=" exp {  }
  | "[" type "]" exp {  }
  ;

arithops:
  exp "*" exp {  }
  | exp "/" exp {  }
  | exp "+" exp {  }
  | exp "-" exp {  }
  ;

logicops:
  exp "==" exp {  }
  | exp "<" exp {  }
  | exp ">" exp {  }
  | exp "&&" exp {  }
  | exp "||" exp {  }
  ;

uop:
  "!" exp { }
  | "-" exp { }
  ;

vdecls:
  vdecl { $$ = new VdeclsNode($1); }
  | vdecls "," vdecl { $$ = new VdeclsNode($1, $3); }
  ;

tdecls:
  type
  | type "," tdecls { }
  ;

vdecl:
  type T_VARID { $$ = new VdeclNode($1, $2); }
  ;

type:
  T_TYPE_INT { $$ = new IntType(); }
  | T_TYPE_CINT { $$ = new CintType(); }
  | T_TYPE_FLOAT { $$ = new FloatType(); }
  | T_TYPE_BOOL { $$ = new BoolType(); }
  | T_TYPE_VOID { $$ = new VoidType(); }
  | T_TYPE_REF type { $$ = new RefType(true); }
  | T_TYPE_NOALIAS T_TYPE_REF type { $$ = new RefType(false); }
  ;

varid:
  T_VARID { }
  ;

globid:
  T_IDENT { }
  ;

lit:
  T_INT_LITERAL {  }
  | T_FLOAT_LITERAL {  }
  | T_BOOL_FALSE_LITERAL {  }
  | T_BOOL_TRUE_LITERAL {  }
  ;


slit:
  T_STRING_LITERAL { }
  ;

%%

int main(int argc, char ** argv) {
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
  ProgramNode * root;
  
  // Parse through the input:
  yyparse(&root);
  
  // Print the AST Tree
  root->PrintRecursive();
}

void yyerror(const char *s) {
  cout << "EEK, parse error!  Message: " << s << endl;
  // might as well halt now:
  exit(-1);
}