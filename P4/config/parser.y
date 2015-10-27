%{
#include <cstdio>
#include <string>
#define DEBUG
#include "util.h"
extern int yylex();
extern int yyerror(const char *msg);
extern int lparent_num;

void missParent(int flag, int line, int column);
%}

%debug
%locations

%left EQ NEQ
%left LT GT LTE GTE
%left PLUS MINUS
%precedence NEG POS
%left MULT DIV MOD

%token CONST INT IF ELSE WHILE VOID ID NUM
%token ASIGN ODD LBRACE RBRACE LBRACKET RBRACKET LPARENT RPARENT COMMA SEMICOLON ERR_RPARENT


%%
CompUnit: Decl 				{debug("CompUnit ::= Decl\n");}
		| FuncDef 			{debug("CompUnit ::= FuncDef\n");}
		| CompUnit Decl 	{debug("CompUnit ::= CompUnit Decl\n");}
		| CompUnit FuncDef 	{debug("CompUnit ::= CompUnit FuncDef\n");}
		;
		
LVal: ID 				{debug("LVal ::= ID\n");}
	| ID LBRACKET Exp RBRACKET {debug("LVal ::= ID '[' Exp ']'\n");}
	;

Exp: LVal 				{debug("Exp ::= LVal\n");}
   | NUM 				{debug("Exp ::= NUM\n");}

   | LPARENT Exp RPARENT {debug("Exp ::= '(' Exp ')'\n");}
   | LPARENT Exp 		{missParent(1, @2.first_line, @2.last_column);}
   | Exp ERR_RPARENT 	{missParent(0, @1.first_line, @1.last_column);}

   | Exp PLUS Exp 		{debug("Exp ::= Exp PLUS Exp\n");}
   | Exp MINUS Exp 		{debug("Exp ::= Exp MINUS Exp\n");}
   | Exp MULT Exp 		{debug("Exp ::= Exp MULT Exp\n");}
   | Exp DIV Exp 		{debug("Exp ::= Exp DIV Exp\n");}
   | Exp MOD Exp 		{debug("Exp ::= Exp MOD Exp\n");}

   | Exp Exp 			{error("missing op, in line %d, column %d.\n", @1.first_line, @1.last_column);}

   | PLUS Exp %prec POS {debug("Exp ::= POS Exp\n");}
   | MINUS Exp %prec NEG {debug("Exp ::= NEG Exp\n");}
   ;

ExpList: Exp 		{debug("ExpList ::= Exp\n");}
	   | ExpList COMMA Exp {debug("ExpList ::= ExpList ',' Exp\n");}
	   ;

Decl: ConstDecl 		{debug("Decl ::= ConstDecl\n");}
	| VarDecl 			{debug("Decl ::= VarDecl\n");}
	;

ConstDecl: CONST INT ConstDefList SEMICOLON {debug("ConstDecl ::= CONST INT ConstDefList ';'\n");}
		 | CONST ConstDefList SEMICOLON {warning("\"ConstDecl ::= CONST ConstDefList ';'\" Expect 'int'\n");} 
		 ;

ConstDefList: ConstDef 	{debug("ConstDefList ::= ConstDef\n");}
			| ConstDefList COMMA ConstDef {debug("ConstDefList ::= ConstDefList ',' ConstDef\n");}
			;

ConstDef: ID ASIGN Exp {debug("ConstDef ::= ID '=' Exp\n");}
		| ID LBRACKET Exp RBRACKET ASIGN LBRACE ExpList RBRACE {debug("ConstDef ::= ID '[' Exp ']' '=' '{' ExpList '}'\n");}
		;

VarDecl: INT VarList SEMICOLON {debug("VarDecl ::= INT VarList ';'\n");}
	   ;

VarList: Var			{debug("VarList ::= Var\n");}
	   | VarList COMMA Var {debug("VarList ::= VarList ',' Var\n");}
	   ;

Var: ID 				{debug("Var ::= ID\n");}
   | ID LBRACKET Exp RBRACKET {debug("Var ::= ID '[' Exp ']'\n");}
   | ID ASIGN Exp 		{debug("Var ::= ID '=' Exp\n");}
   | ID LBRACKET Exp RBRACKET ASIGN LBRACE ExpList RBRACE {debug("Var ::= ID '[' Exp ']' '=' '{' ExpList '}'\n");}
   ;

FuncDef: VOID ID LPARENT RPARENT Block 	{debug("FuncDef ::= VOID ID '(' ')' Block\n");}
	   | VOID ID LPARENT Block 			{missParent(1, @2.first_line, @2.last_column);}
	   | VOID ID ERR_RPARENT Block 		{missParent(0, @1.first_line, @1.last_column);}
	   ;

Block: LBRACE BlockItemList RBRACE {debug("Block ::= '{' BlockItemList '}'\n");}
	 ;

BlockItemList:  
			 | BlockItemList BlockItem {debug("BlockItemList ::= BlockItemList BlockItem\n");}
			 ;

BlockItem: Decl 		{debug("BlockItem ::= Decl\n");}
		 | Stmt 		{debug("BlockItem ::= Stmt\n");}
		 ;

Stmt: LVal ASIGN Exp SEMICOLON {debug("Stmt ::= LVal '=' Exp ';'\n");}

	| ID LPARENT RPARENT SEMICOLON 	{debug("Stmt ::= ID '(' ')' ';'\n");}
	| ID LPARENT SEMICOLON 			{missParent(1, @2.first_line, @2.last_column);}
	| ID ERR_RPARENT SEMICOLON 		{missParent(0, @1.first_line, @1.last_column);}

	| Block 			{debug("Stmt ::= Block\n");}
	
	| IF LPARENT Cond RPARENT Stmt 	{debug("Stmt ::= IF '(' Cond ')' Stmt\n");}
	| IF LPARENT Cond Stmt 			{missParent(1, @2.first_line, @2.last_column);}
	| IF Cond ERR_RPARENT Stmt 		{missParent(0, @1.first_line, @1.last_column);}

	| IF LPARENT Cond RPARENT Stmt ELSE Stmt {debug("Stmt ::= IF '(' Cond ')' Stmt ELSE Stmt\n");}
	| IF LPARENT Cond Stmt ELSE Stmt		{missParent(1, @2.first_line, @2.last_column);}
	| IF Cond ERR_RPARENT Stmt ELSE Stmt	{missParent(0, @1.first_line, @1.last_column);}

	| WHILE LPARENT Cond RPARENT Stmt {debug("Stmt ::= WHILE '(' Cond ')' Stmt\n");}
	| WHILE LPARENT Cond Stmt		{missParent(1, @2.first_line, @2.last_column);}
	| WHILE Cond ERR_RPARENT Stmt 	{missParent(0, @1.first_line, @1.last_column);}

	| SEMICOLON 		{debug("Stmt ::= ';'\n");}
	;

Cond: ODD Exp 			{debug("Cond ::= ODD Exp\n");}
	| Exp LT Exp 		{debug("Cond ::= Exp LT Exp\n");}
	| Exp GT Exp 		{debug("Cond ::= Exp GT Exp\n");}
	| Exp LTE Exp 		{debug("Cond ::= Exp LTE Exp\n");}
	| Exp GTE Exp 		{debug("Cond ::= Exp GTE Exp\n");}
	| Exp EQ Exp 		{debug("Cond ::= Exp EQ Exp\n");}
	| Exp NEQ Exp 		{debug("Cond ::= Exp NEQ Exp\n");}
	;


%%

int yyerror(const char *msg)
{
	printf("%s", msg);
	return 0;
}

void missParent(int flag, int line, int column)
{
	char missing = flag ? ')' : '(';
	int inc = flag ? -1 : 1;
	lparent_num += inc; 
	error("Expect '%c', in line %d, column %d\n", missing, line, column);
}



