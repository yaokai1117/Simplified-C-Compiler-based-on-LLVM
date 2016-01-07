/* Scanner for simple C1 source code */

%{
#include <cctype>
#include <cstdlib>
#include <string>
#include "node.h"
#include "tok.h"

int yycolumn = 1;
int lparent_num = 0;

#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno; \
	yylloc.first_column = yycolumn; yylloc.last_column = yycolumn + yyleng - 1; \
	yycolumn += yyleng;

%}

	/*definitions*/
delim 		[ \t]
blockcomment 	\/\*([^\*]|((\*)*[^/\*]))*(\*)+\/ 
linecomment  	\/\/.*\n
ws 			{delim}+
letter 		[A-Za-z_]
digit 		[0-9]
id 			{letter}({letter}|{digit})*
num 		{digit}+
fnum 		{digit}+\.{digit}+
char 		'[^\\']'|'\\n'|'\\t'|'\\\\'|'\\''

%option noyywrap
%option yylineno

%%
	/* rules */ 

{ws} 		{/*do nothing*/}
{blockcomment} 	{
					int cnt = 0;
					int newline_flag = 0;
					for (int i = 0; yytext[i] != '\0'; i++) {
						if (yytext[i] == '\n') {
							newline_flag = 1;
							cnt = 0;
						}
						cnt++;
					}
					if (newline_flag == 1)
						yycolumn = cnt;
				}
{linecomment} 	{}
const 		{return CONST;}
int 		{return INTTYPE;}
float 		{return FLOATTYPE;}
char 		{return CHARTYPE;}
if 			{return IF;}
else 		{return ELSE;}
while  		{return WHILE;}
return 		{return RETURN;}
break 		{return BREAK;}
continue 	{return CONTINUE;}
void 		{return VOID;}
extern 		{return EXTERN;}
static 		{return STATIC;}
{id} 		{
				yylval.name = new std::string(yytext);
				return ID;
			}
{num} 		{
				yylval.ival = atoi(yytext);
				return NUM;
			}
{fnum} 		{
				yylval.fval = strtod(yytext, 0);
				return FNUM;
			}
{char} 		{
				if (yytext[1] != '\\') {
					yylval.cval = yytext[1];
				}
				else {
					switch (yytext[2]) {
					case 'n':
						yylval.cval = '\n';
						break;
					case '\\':
						yylval.cval = '\\';
						break;
					case 't':
						yylval.cval = '\t';
						break;
					default:
						yylval.cval = 0;
						break;
					}
				}
				return CHAR;
			}
"+" 		{return PLUS;}
"-" 		{return MINUS;}
"*" 		{return MULT;}
"/" 		{return DIV;}
"%" 		{return MOD;}
"&" 		{return SINGLE_AND;}
"{" 		{return LBRACE;}
"}" 		{return RBRACE;}
"[" 		{return LBRACKET;}
"]" 		{return RBRACKET;}
"(" 		{lparent_num++; return LPARENT;}
")" 		{
				lparent_num--;
				if (lparent_num >= 0) 
					return RPARENT;
				else 
					return ERR_RPARENT;
			}
"=" 		{return ASIGN;}
"," 		{return COMMA;}
";" 		{return SEMICOLON;}
"<" 		{return LT;}
">" 		{return GT;}
"<=" 		{return LTE;}
">=" 		{return GTE;}
"!=" 		{return NEQ;}
"==" 		{return EQ;}
"!" 		{return NOT;}
"&&" 		{return AND;}
"||" 		{return OR;}

\n 			{yycolumn = 1;}

%%

