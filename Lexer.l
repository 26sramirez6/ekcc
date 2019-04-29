%{
  #include "ValidTypes.hpp"
  #include "Expression.h"
  #include "Parser.hpp"
  #include <iostream>
  using namespace std;
%}

%%
[ \t\n]           { continue; }
"int"             { cout << "type: " << yytext << endl; yylval.sval = strdup(yytext); return T_TYPE_INT; }
"float"           { cout << "type: " << yytext << endl; return T_TYPE_FLOAT; }
"bool"            { cout << "type: " << yytext << endl; return T_TYPE_BOOL; }
"cint"            { cout << "type: " << yytext << endl; return T_TYPE_CINT; }
"void"            { cout << "type: " << yytext << endl; return T_TYPE_VOID; }
"if"              { cout << "control_flow: " << yytext << endl; return T_CONTROL_IF; }
"else"            { cout << "control_flow: " << yytext << endl; return T_CONTROL_ELSE; }
"while"           { cout << "control_flow: " << yytext << endl; return T_CONTROL_WHILE; }
"for"             { cout << "control_flow: " << yytext << endl; return T_CONTROL_FOR; }
"return"          { cout << "control_flow: " << yytext << endl; return T_CONTROL_RETURN; }

"print"           { cout << "function: " << yytext << endl; return T_FUNCTION_PRINT; }
"run"             { cout << "function: " << yytext << endl; return T_FUNCTION_RUN; }
"def"             { cout << "function: " << yytext << endl; return T_FUNCTION_DEF; }
"extern"          { cout << "function: " << yytext << endl; return T_FUNCTION_EXTERN; }

\"[^\n"]+\"     { cout << "String" << yytext << endl; return T_STRING_LITERAL; }
[0-9]+          { cout << "INT" << yytext << endl; return T_INT_LITERAL; }
[0-9]+\.[0-9]+  { cout << "FLOAT" << yytext << endl; return T_FLOAT_LITERAL; }

"*"          { return T_STAR; }
"+"          { return T_PLUS; }
"("          { return T_LPAREN; }
")"          { return T_RPAREN; }

.            { continue; /* Ignore unexpected characters. */}

%%

//((noalias)? ref) { cout << "type: " << yytext << endl; yyval.Init(yytext); return T_TYPE_REF; }



