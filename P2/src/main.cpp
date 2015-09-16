#include "tok.h"
#include <cstdio>
#include <cstdlib>

extern FILE* yyin;
extern int yylex();

extern void print_token(int token, FILE* fp);

int main()
{
	FILE* infile;
	char infilename[100];
	printf("Input the file name of C1 source\n");
	scanf("%s", infilename);
	
	if ((infile = fopen(infilename, "r")) == NULL) {
		printf("File %s can't be opened.\n", infilename);
		exit(1);
	}

	yyin = infile;
	int token = yylex();
	while (token) {
		print_token(token, stdout);
		token = yylex();
	}
	return 0;
}
