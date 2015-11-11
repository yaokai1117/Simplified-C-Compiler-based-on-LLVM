%{
#include <cstdio>
#include <string>
#define DEBUG
#include "util.h"
#include "global.h"
#include "msgfactory.h"

extern int yylex();
extern int yyerror(const char *msg);
extern int lparent_num;

MsgFactory msgFactory;

%}

%locations
%initial-action 
{
    msgFactory.initial(infile_name);	
};



%token CONST INT IF ELSE WHILE VOID ID NUM
%token ASIGN LBRACE RBRACE LBRACKET RBRACKET LPARENT RPARENT COMMA SEMICOLON ERR_RPARENT 

%precedence NO_ELSE
%precedence ELSE

%precedence MISSING_RPARENT
%precedence RPARENT LPARENT ERR_RPARENT

%left error  /*not good, but useful here*/
%left EQ NEQ
%left LT GT LTE GTE
%left PLUS MINUS
%left MULT DIV MOD

%precedence NEG POS

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
   | LPARENT Exp %prec MISSING_RPARENT		
   		{
			lparent_num--;
   			Error err = msgFactory.newError(e_rparent, @2.last_line, @2.last_column);
			msgFactory.showMsg(&err);
			debug("Exp ::= '(' Exp ')' 		[error recovery]\n");
   		}
   | Exp ERR_RPARENT 	
   		{
			lparent_num++;
   			Error err = msgFactory.newError(e_lparent, @1.first_line, @1.first_column);
			msgFactory.showMsg(&err);
			debug("Exp ::= '(' Exp ')' 		[error recovery]\n");
   		}

   | Exp PLUS Exp 		{debug("Exp ::= Exp PLUS Exp\n");}
   | Exp MINUS Exp 		{debug("Exp ::= Exp MINUS Exp\n");}
   | Exp MULT Exp 		{debug("Exp ::= Exp MULT Exp\n");}
   | Exp DIV Exp 		{debug("Exp ::= Exp DIV Exp\n");}
   | Exp MOD Exp 		{debug("Exp ::= Exp MOD Exp\n");}

   | Exp error  Exp 	
   		{
   			yyerrok; 
   			Error err = msgFactory.newError(e_miss_op, @2.first_line, @2.last_column - 1);
			msgFactory.showMsg(&err);
			debug("Exp ::= Exp Exp 		[error recovery]\n");
		}

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
		 | CONST ConstDefList SEMICOLON 
		 	{
				Warning warn = msgFactory.newWarning(w_miss_int, @2.first_line, @2.first_column);
				msgFactory.showMsg(&warn);
				debug("ConstDecl ::= CONST INT ConstDefList ';' 	[error recovery]\n");
			}
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


	| ID LPARENT RPARENT SEMICOLON {debug("Stmt ::= ID '(' ')' ';'\n");}

	| Block 			{debug("Stmt ::= Block\n");}
	
	| IF LPARENT Cond RPARENT Stmt %prec NO_ELSE	{debug("Stmt ::= IF '(' Cond ')' Stmt\n");}

	| IF LPARENT Cond RPARENT Stmt ELSE Stmt  {debug("Stmt ::= IF '(' Cond ')' Stmt ELSE Stmt\n");}

	| WHILE LPARENT Cond RPARENT Stmt {debug("Stmt ::= WHILE '(' Cond ')' Stmt\n");}

	| SEMICOLON 		{debug("Stmt ::= ';'\n");}
	;

Cond: Exp LT Exp 		{debug("Cond ::= Exp LT Exp\n");}
	| Exp GT Exp 		{debug("Cond ::= Exp GT Exp\n");}
	| Exp LTE Exp 		{debug("Cond ::= Exp LTE Exp\n");}
	| Exp GTE Exp 		{debug("Cond ::= Exp GTE Exp\n");}
	| Exp EQ Exp 		{debug("Cond ::= Exp EQ Exp\n");}
	| Exp NEQ Exp 		{debug("Cond ::= Exp NEQ Exp\n");}
	;


%%

int yyerror(const char *msg)
{
	printf("%s\n", msg);
	return 0;
}


