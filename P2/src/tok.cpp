#include <string>
#include <cstdio>
#include "tok.h"

std::string var_val;
int num_val;
int op_val;

void print_token(int token, FILE* fp)
{
	switch (token) {
		case ID: 	fprintf(fp, "<ID, %s>", var_val.c_str()); break;
		case NUM: 	fprintf(fp, "<NUM, %d>", num_val); break;
		case OP: 	fprintf(fp, "<OP, %c>", (char) op_val); break;
		case CONST: 	fprintf(fp, "<CONST, >"); break;
		case INTDEF: 	fprintf(fp, "<INTDEF, >"); break;
		case IF: 	fprintf(fp, "<IF, >"); break;
		case ELSE: 	fprintf(fp, "<ELSE, >"); break;
		case WHILE: fprintf(fp, "<WHILE, >"); break;
		case ODD: 	fprintf(fp, "<ODD, >"); break;
		case LBRACE: 	fprintf(fp, "<LBRACE, >"); break;
		case RBRACE: 	fprintf(fp, "<RBRACE, >"); break;
		case LBRACKET: 	fprintf(fp, "<LBRACKET, >"); break;
		case RBRACKET: 	fprintf(fp, "<RBRACKET, >"); break;
		case LPARENT: 	fprintf(fp, "<LPARENT, >"); break;
		case RPARENT: 	fprintf(fp, "<RPARENT, >"); break;
		case EQUAL: 	fprintf(fp, "<EQUAL, >"); break;
		case COMMA: 	fprintf(fp, "<COMMA, >"); break;
		case SEMICOLON: fprintf(fp, "<SEMICOLON, >"); break;
		case RELOP: 	fprintf(fp, "<RELOP, %d>", op_val); break;
		case QUOT: 	fprintf(fp, "<QUOT, >"); break;
		case SLASH: fprintf(fp, "<SLASH, >"); break;
		default: break;
	}

	printf("\n");
}
