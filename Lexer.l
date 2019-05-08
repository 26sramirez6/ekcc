%{
  #include "ValidTypes.hpp"
  #include "AST.hpp"
  #include "Parser.hpp"
  #include <iostream>
  using namespace std;
  #define YY_DECL extern "C" int yylex()
  int lineNumber = 1;
%}

%%
[ \t]			  { continue; }
[\n]              { lineNumber++; continue; }
"#".*             { continue; }
"int"             { return T_TYPE_INT; }
"float"           { return T_TYPE_FLOAT; }
"bool"            { return T_TYPE_BOOL; }
"cint"            { return T_TYPE_CINT; }
"void"            { return T_TYPE_VOID; }
"noalias"         { return T_TYPE_NOALIAS; }
"ref"             { return T_TYPE_REF;}

"true"            { return T_BOOL_TRUE_LITERAL; }
"false"           { return T_BOOL_FALSE_LITERAL; }

"if"              { return T_CONTROL_IF; }
"else"            { return T_CONTROL_ELSE; }
"while"           { return T_CONTROL_WHILE; }
"for"             { return T_CONTROL_FOR; }
"return"          { return T_CONTROL_RETURN; }

"print"           { return T_FUNCTION_PRINT; }
"def"             { return T_FUNCTION_DEF; }
"extern"          { return T_FUNCTION_EXTERN; }

\$[a-zA-Z_]+[a-zA-Z0-9_]* { yylval.sval = strdup(yytext); return T_VARID;}
[a-zA-Z_]+[a-zA-Z0-9_]* { yylval.sval = strdup(yytext); return T_IDENT; }
\"[^\n"]+\"    { yylval.sval = strdup(yytext); return T_STRING_LITERAL; }
[0-9]+          { yylval.ival = atoi(yytext); return T_INT_LITERAL; }
[0-9]+\.[0-9]+  { yylval.fval = atof(yytext); return T_FLOAT_LITERAL; }

"*"           { return T_STAR; }
"+"           { return T_PLUS; }
"-"           { return T_HYPHEN; }
"/"           { return T_FSLASH; }

"="            { return T_ASSIGN;}

">"           { return T_GT; }
"<"           { return T_LT; }
"||"          { return T_OR; }
"&&"          { return T_AND; }
"=="          { return T_EQ; }
"!"           { return T_NEG; }

"{"           { return T_LBRACE; }
"}"           { return T_RBRACE; }
"("           { return T_LPAREN; }
")"           { return T_RPAREN; }
"["           { return T_LBRACKET; }
"]"           { return T_RBRACKET; }
","           { return T_COMMA;}
";"           { return T_SEMICOLON;}
%%




