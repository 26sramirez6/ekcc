%{
  #include "ValidTypes.hpp"
  #include "Expression.h"
  #include "Parser.hpp"
  #include <iostream>
  using namespace std;
%}

%%
[ \t\n]           { continue; }
"int"             { cout << "type: " << yytext << endl; return T_TYPE_INT; }
"float"           { cout << "type: " << yytext << endl; return T_TYPE_FLOAT; }
"bool"            { cout << "type: " << yytext << endl; return T_TYPE_BOOL; }
"cint"            { cout << "type: " << yytext << endl; return T_TYPE_CINT; }
"void"            { cout << "type: " << yytext << endl; return T_TYPE_VOID; }
"noalias"         { return T_TYPE_NOALIAS; }
"ref"             { return T_TYPE_REF;}

"true"            { return T_BOOL_TRUE_LITERAL; }
"false"           { return T_BOOL_FALSE_LITERAL; }

"if"              { cout << "control_flow: " << yytext << endl; return T_CONTROL_IF; }
"else"            { cout << "control_flow: " << yytext << endl; return T_CONTROL_ELSE; }
"while"           { cout << "control_flow: " << yytext << endl; return T_CONTROL_WHILE; }
"for"             { cout << "control_flow: " << yytext << endl; return T_CONTROL_FOR; }
"return"          { cout << "control_flow: " << yytext << endl; return T_CONTROL_RETURN; }

"print"           { cout << "function: " << yytext << endl; return T_FUNCTION_PRINT; }
"run"             { cout << "function: " << yytext << endl; return T_FUNCTION_RUN; }
"def"             { cout << "function: " << yytext << endl; return T_FUNCTION_DEF; }
"extern"          { cout << "function: " << yytext << endl; return T_FUNCTION_EXTERN; }

\$[a-zA-Z_]+[a-zA-Z0-9_]* { cout << "variable: " << yytext << endl; return T_VARID;}
[a-zA-Z_]+[a-zA-Z0-9_]* { cout << "identifier: " << yytext << endl; return T_IDENT; }
\"[^\n"]+\"     { cout << "String" << yytext << endl; return T_STRING_LITERAL; }
[0-9]+          { cout << "INT" << yytext << endl; return T_INT_LITERAL; }
[0-9]+\.[0-9]+  { cout << "FLOAT" << yytext << endl; return T_FLOAT_LITERAL; }

"*"           { cout << "symbol: " << yytext << endl; return T_STAR; }
"+"           { cout << "symbol: " << yytext << endl; return T_PLUS; }
"-"           { cout << "symbol: " << yytext << endl; return T_HYPHEN; }
"/"           { cout << "symbol: " << yytext << endl; return T_FSLASH; }

"="            { cout << "symbol: " << yytext << endl; return T_ASSIGN;}

">"           { cout << "symbol: " << yytext << endl; return T_GT; }
"<"           { cout << "symbol: " << yytext << endl; return T_LT; }
"||"          { cout << "symbol: " << yytext << endl; return T_OR; }
"&&"          { cout << "symbol: " << yytext << endl; return T_AND; }
"=="          { cout << "symbol: " << yytext << endl; return T_EQ; }
"!"           { cout << "symbol: " << yytext << endl; return T_NEG; }

"{"           { cout << "symbol: " << yytext << endl; return T_LBRACE; }
"}"           { cout << "symbol: " << yytext << endl; return T_RBRACE; }
"("           { cout << "symbol: " << yytext << endl; return T_LPAREN; }
")"           { cout << "symbol: " << yytext << endl; return T_RPAREN; }
"["           { cout << "symbol: " << yytext << endl; return T_LBRACKET; }
"]"           { cout << "symbol: " << yytext << endl; return T_RBRACKET; }
"#"           { cout << "symbol: " << yytext << endl; return T_POUND; }
","           { cout << "symbol: " << yytext << endl; return T_COMMA;}
";"           { cout << "symbol: " << yytext << endl; return T_SEMICOLON;}


%%

//((noalias)? ref) { cout << "type: " << yytext << endl; yyval.Init(yytext); return T_TYPE_REF; }
//"$"           { return T_DOLLAR; }



