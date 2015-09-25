/* Scanner for simple C1 source code */

%{
#include "tok.h"
#include <cctype>
%}

	/*definitions*/
delim 		[ \t\n]
ws 			{delim}+
letter 		[A-Za-z]
digit 		[0-9]
id 			{letter}({letter}|{digit})*
num 		{digit}+
op 			[+\-\*/%]


%%
	/* rules */ 

{ws} 		{/*do nothing*/}
const 		{return CONST;}
int 		{return INTDEF;}
if 			{return IF;}
else 		{return ELSE;}
while  		{return WHILE;}
{id} 		{var_val = (std::string) yytext; return ID;}
{num} 		{num_val = atoi(yytext); return NUM;}
{op} 		{op_val = yytext[0]; return OP;}
"!" 		{return ODD;}
"{" 		{return LBRACE;}
"}" 		{return RBRACE;}
"[" 		{return LBRACKET;}
"]" 		{return RBRACKET;}
"(" 		{return LPARENT;}
")" 		{return RPARENT;}
"=" 		{return EQUAL;}
"," 		{return COMMA;}
";" 		{return SEMICOLON;}
"<" 		{op_val = LT; return RELOP;}
">" 		{op_val = GT; return RELOP;}
"<=" 		{op_val = LTE; return RELOP;}
">=" 		{op_val = GTE; return RELOP;}
"!=" 		{op_val = NEQ; return RELOP;}
"==" 		{op_val = EQ; return RELOP;}
"\"" 		{return QUOT;}
"\\" 		{return SLASH;}

%%

	/* user code */

int yywrap()
{
	return 1;
}
