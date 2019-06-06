%{
    #include <stdio.h>
    #include <unistd.h>
    #include <sys/wait.h>
    #include <cstdio>
    #include <iostream>
    #include <string>
    #include <fstream>
    #include <sstream>
    #include <llvm/Support/TargetSelect.h>
    #include <llvm/Support/InitLLVM.h>
    #include "CompilerConfig.hpp"
    #include "ValidTypes.hpp"
    #include "AST.hpp"
    #include "Parser.hpp"
    #include "llvm/IR/Function.h"
    using namespace std;
    // stuff from flex that bison needs to know about:
    extern "C" {
    extern int yylex();
    extern int yyparse();
    extern int lineNumber;
    extern FILE *yyin;
  
    void yyerror(ProgramNode ** node, const char *s) {}
    }

%}

%code requires {
    #include <stdio.h>
    #include <unistd.h>
    #include <sys/wait.h>
    #include <cstdio>
    #include <iostream>
    #include <string>
    #include <fstream>
    #include <sstream>
    #include <llvm/Support/TargetSelect.h>
    #include <llvm/Support/InitLLVM.h>
    #include "CompilerConfig.hpp"
    #include "ValidTypes.hpp"
    #include "AST.hpp"
    #include "llvm/IR/Function.h"
}

%union{
  int ival;
  bool bval;
  float fval;
  char *sval;

  Literal * literal;
  ControlFlow * controlFlow;
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
  StatementNode * statementNode;
  StatementsNode * statementsNode;
  ExpressionNode * expressionNode;
  ExpressionsNode * expressionsNode;
  BinaryOperationNode * binaryOperationNode;
  UnaryOperationNode * unaryOperationNode;
  ExistingVarNode * existingVarNode;
  ExistingFuncNode * existingFuncNode;
  TdeclsNode * tdeclsNode;
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
%type <literal> lit
%type <validType> type
%type <programNode> prog
%type <funcsNode> funcs
%type <externsNode> externs
%type <vdeclNode> vdecl
%type <vdeclsNode> vdecls
%type <funcNode> func
%type <externNode> extern
%type <blockNode> blk
%type <statementsNode> stmts
%type <statementNode> stmt
%type <expressionsNode> exps
%type <expressionNode> exp
%type <binaryOperationNode> binop
%type <binaryOperationNode> arithops
%type <binaryOperationNode> logicops
%type <unaryOperationNode> uop
%type <existingVarNode> varid
%type <existingFuncNode> globid
%type <tdeclsNode> tdecls

%parse-param { ProgramNode ** root }

%left T_ASSIGN
%left T_OR
%left T_AND
%left T_EQ
%left T_GT T_LT
%left T_PLUS T_HYPHEN
%left T_STAR T_FSLASH
%right T_NEG
%nonassoc UMINUS
%right T_RBRACKET

%%

prog:
  funcs { $$ = new ProgramNode(lineNumber, $1); (*root) = $$; }
  | externs funcs { $$ = new ProgramNode(lineNumber, $1, $2); (*root) = $$; }
  ;

externs:
  extern { $$ = new ExternsNode(lineNumber,$1); }
  | externs extern { $$ = new ExternsNode(lineNumber,$1, $2); }
  ;

extern:
  T_FUNCTION_EXTERN type T_IDENT "(" ")" ";" { 
	$$ = new ExternNode(lineNumber,$2, $3); 
	}
  | T_FUNCTION_EXTERN type T_IDENT "(" tdecls ")" ";" { 
	  $$ = new ExternNode(lineNumber,$2, $3, $5); 
  	}
  ;

funcs:
  func { $$ = new FuncsNode(lineNumber,$1); }
  | funcs func { $$ = new FuncsNode(lineNumber,$1, $2); }
  ;

func:
  T_FUNCTION_DEF type T_IDENT "(" ")" blk { $$ = new FuncNode(lineNumber,$2, $3, $6); }
  | T_FUNCTION_DEF type T_IDENT "(" vdecls ")" blk { $$ = new FuncNode(lineNumber,$2, $3, $5, $7); }
  ;

blk:
  "{" "}" { $$ = new BlockNode(lineNumber); }
  | "{" stmts "}" { $$ = new BlockNode(lineNumber,$2); }
  ; 

stmts:
  stmt { $$ = new StatementsNode(lineNumber, $1); }
  | stmts stmt { $$ = new StatementsNode(lineNumber, $1, $2); }
  ;

stmt:
  blk { $$ = new StatementNode(lineNumber,$1); }
  | T_CONTROL_RETURN ";" { $$ = new StatementNode(lineNumber,new ReturnControl()); }
  | T_CONTROL_RETURN exp ";" { $$ = new StatementNode(lineNumber,new ReturnControl(), $2); }
  | vdecl "=" exp ";" { $$ = new StatementNode(lineNumber,$1, $3); }
  | exp ";" { $$ = new StatementNode(lineNumber,$1); }
  | T_CONTROL_WHILE "(" exp ")" stmt { $$ = new StatementNode(lineNumber,new WhileControl(), $3, $5); }
  | T_CONTROL_IF "(" exp ")" stmt { $$ = new StatementNode(lineNumber,new IfControl(), $3, $5); }
  | T_CONTROL_IF "(" exp ")" stmt  T_CONTROL_ELSE stmt { 
	  $$ = new StatementNode(lineNumber,new ElseControl(), $3, $5, $7);
  }
  | T_FUNCTION_PRINT exp ";" { $$ = new StatementNode(lineNumber,new PrintFunction(), $2); }
  | T_FUNCTION_PRINT T_STRING_LITERAL ";" { $$ = new StatementNode(lineNumber,new PrintFunction(), $2); }
  ;

exps:
  exp { $$ = new ExpressionsNode(lineNumber,$1); }
  | exps "," exp { $$ = new ExpressionsNode(lineNumber,$1, $3); }
  ;

exp:
  "(" exp ")" { $$ = new ExpressionNode(lineNumber,$2); }
  | binop { $$ = new ExpressionNode(lineNumber,$1); }
  | uop { $$ = new ExpressionNode(lineNumber,$1); }
  | lit { $$ = new ExpressionNode(lineNumber,$1); }
  | varid { $$ = new ExpressionNode(lineNumber,$1); }
  | globid "(" ")" { $$ = new ExpressionNode(lineNumber,$1); }
  | globid "(" exps ")" { $$ = new ExpressionNode(lineNumber,$1, $3); }
  ;

binop:
  arithops { $$ = $1; }
  | logicops { $$ = $1; }
  | varid "=" exp { $$ = new BinaryOperationNode(lineNumber,Assign, $1, $3); }
  | "[" type "]" exp { $$ = new BinaryOperationNode(lineNumber,Cast, $2, $4); }
  ;

logicops:
  exp "==" exp { $$ = new BinaryOperationNode(lineNumber,Equality, $1, $3); }
  | exp "<" exp { $$ = new BinaryOperationNode(lineNumber,LessThan, $1, $3); }
  | exp ">" exp { $$ = new BinaryOperationNode(lineNumber,GreaterThan, $1, $3); }
  | exp "&&" exp { $$ = new BinaryOperationNode(lineNumber,Land, $1, $3); }
  | exp "||" exp { $$ = new BinaryOperationNode(lineNumber,Lor, $1, $3); }
  ;

arithops:
  exp "*" exp { $$ = new BinaryOperationNode(lineNumber,Multiply, $1, $3); }
  | exp "/" exp { $$ = new BinaryOperationNode(lineNumber,Divide, $1, $3); }
  | exp "+" exp { $$ = new BinaryOperationNode(lineNumber,Add, $1, $3);  }
  | exp "-" exp { $$ = new BinaryOperationNode(lineNumber,Subtract, $1, $3); }
  ;
    
uop:
  "!" exp { $$ = new UnaryOperationNode(lineNumber,Not, $2); }
  | "-" exp %prec UMINUS{ $$ = new UnaryOperationNode(lineNumber,Minus, $2); }
  ;

vdecls:
  vdecl { $$ = new VdeclsNode(lineNumber,$1); }
  | vdecls "," vdecl { $$ = new VdeclsNode(lineNumber,$1, $3); }
  ;

tdecls:
  type { $$ = new TdeclsNode(lineNumber,$1); }
  | tdecls "," type { $$ = new TdeclsNode(lineNumber,$1, $3); }
  ;

vdecl:
  type T_VARID { $$ = new VdeclNode(lineNumber, $1, $2); }
  ;

type:
  T_TYPE_INT { $$ = new IntType(); }
  | T_TYPE_CINT { $$ = new CintType(); }
  | T_TYPE_FLOAT { $$ = new FloatType(); }
  | T_TYPE_BOOL { $$ = new BoolType(); }
  | T_TYPE_VOID { $$ = new VoidType(); }
  | T_TYPE_REF { $$ = new RefType(); }
  | T_TYPE_REF type { $$ = new RefType(false, $2); }
  | T_TYPE_NOALIAS T_TYPE_REF type { $$ = new RefType(true, $3); }
  ;

varid:
  T_VARID { $$ = new ExistingVarNode(lineNumber,$1); }
  ;

globid:
  T_IDENT { $$ = new ExistingFuncNode(lineNumber,$1); }
  ;

lit:
  T_INT_LITERAL { $$ = new Literal($1); }
  | T_FLOAT_LITERAL { $$ = new Literal($1); }
  | T_BOOL_FALSE_LITERAL { $$ = new Literal(false); }
  | T_BOOL_TRUE_LITERAL { $$ = new Literal(true); }
  ;

%%

// initialize static AST members
ASTNode * ASTNode::root_ = nullptr;
bool ASTNode::ready_ = false;
vector<string> ASTNode::compilerErrors_;
vector<int> ASTNode::lineNumberErrors_;
VarTable ASTNode::varTable_;
FuncTable ASTNode::funcTable_;
tuple<string, int> ASTNode::recursiveFuncPlaceHolder_ = make_tuple("", -1);
bool ASTNode::runDefined_ = false;
llvm::Function * ASTNode::runFunction_ = nullptr;
llvm::Function * ASTNode::printfFunction_ = nullptr;
llvm::Function * ASTNode::cintAddFunction_ = nullptr;
llvm::Function * ASTNode::cintMultiplyFunction_ = nullptr;
llvm::Function * ASTNode::cintSubtractFunction_ = nullptr;
llvm::Function * ASTNode::cintDivideFunction_ = nullptr;
llvm::Function * ASTNode::cintNegateFunction_ = nullptr;
// static assert that CLANG_BINARY defined
#define Q(x) #x
#define QUOTE(x) Q(x)
#ifdef CLANG_BINARY
#define QUOTED_CLANG_BINARY QUOTE(CLANG_BINARY)
#endif

#ifndef QUOTED_CLANG_BINARY 
	static_assert(false, "QOUTED_CLANG_BINARY not defined");
#endif

	
int 
main(int argc, char ** argv) {
	
	CompilerConfig cfg(argc, argv);
	if (!cfg.properConfig_) {
		cfg.PrintUsage();
		return 1;
	} else if (cfg.help_) {
		cfg.PrintUsage();
		return 0;
	}

  //if (cfg.jit_) llvm::InitLLVM X(argc, argv);

  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

	// open a file handle to a particular file:
	FILE * unit = fopen(cfg.inputFile_, "r");
	// make sure it's valid:
	if (!unit) {
		cout << "error: could not open " 
				<< cfg.inputFile_ << endl;
		return -1;
	}
	// Set flex to read from it instead of defaulting to STDIN:
	yyin = unit;
	
	// Set up the root tree
	ProgramNode * root;
	ASTNode::StaticInit(cfg.inputFile_);
	
	// Parse through the input:
	yyparse(&root);
	
	// Print the AST Tree
	if (cfg.emitAST_) {
		// set up the string stream
		stringstream ssAST;
		root->PrintRecursive(ssAST, 0);
		cout << ssAST.str() << "\n...";
	}
	
	if (root->HasCompilerErrors()) {
		cout << root->GetCompilerErrors() << endl;
		return 1;
	}
	
	string stringIR("");
	llvm::raw_string_ostream ssIR(stringIR);
  root->GenerateCodeRecursive(ssIR, cfg.optimize_);
 
	if (cfg.emitLLVM_) {
		cout << ssIR.str() << endl;
	}
	
	if (!cfg.outputFile_.empty() && !cfg.jit_) {

		ofstream out(cfg.outputFile_ + ".ll");
		out << ssIR.str() << endl;
		out.close();
		
		int rv = fork();
		if (rv < 0) {
			cerr << "error: fork failed\n";
			return 1;
		} else if (rv==0) { //child
			const char * args[6];
			args[0] = strdup(QUOTED_CLANG_BINARY);
			args[1] = (cfg.outputFile_ + ".ll").c_str();
			args[2] = "main.ll";
			args[3] = strdup("-o");
			args[4] = cfg.outputFile_.c_str();
			args[5] = NULL;
//			cout << "child executing: " <<
//				args[0] << " " <<
//				args[1] << " " <<
//				args[2] << " " <<
//				args[3] << " " <<
//				args[4] << endl;
			execvp((const char *)args[0], (char* const*)args);
		} else { // parent
			wait(NULL);
		}
	}	else if (cfg.jit_) {
    root->ExecuteJIT();
  }
	
	
	
	return 0;
}

void yyerror(const char *s) {
  cout << "compile error" << s << endl;
  exit(1);
}