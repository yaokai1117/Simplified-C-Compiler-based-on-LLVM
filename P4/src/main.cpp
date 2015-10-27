#include <cstdio>

extern int yylex();
extern int yyparse();
extern int yydebug;

int main()
{
	yyparse();
	return 0;
}
