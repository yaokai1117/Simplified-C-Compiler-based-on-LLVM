%{
#include <cstdio>
#include <string>
#include <list>
#include "util.h"
#include "global.h"
#include "msgfactory.h"
#include "node.h"

extern int yylex();
extern int yyerror(const char *msg);
extern int lparent_num;

extern MsgFactory msgFactory;

extern CompUnitNode *root;
extern bool errorFlag;
extern list<Node*> astNodes;

void clearAstNodes();

%}

%locations
%initial-action 
{
    msgFactory.initial(infile_name);	
};

%union
{
	int ival;
	double fval;
	char cval;
	std::string *name;
	Node *node;
	NodeList *nodeList;
}


%token CONST INTTYPE FLOATTYPE CHARTYPE EXTERN 
%token IF ELSE WHILE VOID ID NUM FNUM CHAR BREAK CONTINUE
%token ASIGN LBRACE RBRACE LBRACKET RBRACKET LPARENT RPARENT 
%token COMMA SEMICOLON ERR_RPARENT 

%precedence NO_ELSE
%precedence ELSE

%precedence MISSING_RPARENT
%precedence RPARENT LPARENT ERR_RPARENT

%left error  /*not good, but useful here*/
%left OR
%left AND
%left NOT
%left EQ NEQ
%left LT GT LTE GTE
%left PLUS MINUS
%left MULT DIV MOD

%precedence NEG POS

%type <ival> NUM
%type <fval> FNUM
%type <cval> CHAR
%type <name> ID
%type <node> CompUnit CompUnitItem ExternDecl FuncDecl FuncDef LVal Exp Decl ConstDecl VarDecl ConstDef Var 
%type <node> Block BlockItem Stmt Cond
%type <nodeList> ExpList ConstDefList VarList BlockItemList ArgNameList

%destructor {
	delete ($$);
} ID

%%


CompUnit: CompUnitItem 				
			{
				if (!errorFlag) {
					root = new CompUnitNode($1);
					root->setLoc((Loc*)&(@$));
					astNodes.push_back(root);
				}
			}
		| CompUnit CompUnitItem 	
			{
				if (!errorFlag) {
					root->append($2);
					root->setLoc((Loc*)&(@$));
				}
			}
		;

CompUnitItem: Decl 				
			{
				if (!errorFlag) {
					$$ = $1;
				}
			}
		| FuncDef 			
			{
				if (!errorFlag) {
					$$ = $1;
				}
			}
		| ExternDecl
			{
				if (!errorFlag) {
					$$ = $1;
				}
			}
		| FuncDecl		 			
			{
				if (!errorFlag) {
					$$ = $1;
				}
			}
		;

		
LVal: ID 				
		{
			if (!errorFlag) {
				$$ = new IdNode($1);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
			else {
				delete $1;
			}
		}
	| ID LBRACKET Exp RBRACKET 
		{
			if (!errorFlag) {
				$$ = new ArrayItemNode($1, (ExpNode*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
			else {
				delete $1;
			}
		}
	;

Exp: LVal 				
   		{
			if (!errorFlag) {
				$$ = $1;
			}
		}
   | NUM 				
   		{
			if (!errorFlag) {
				$$ = new NumNode($1);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
   | FNUM
   		{
			if (!errorFlag) {
				$$ = new FNumNode($1);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
   | CHAR
   		{
			if (!errorFlag) {
				$$ = new CharNode($1);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
   | LPARENT Exp RPARENT 
   		{
			if (!errorFlag) {
				$$ = $2;
				$$->setLoc((Loc*)&(@$));
			}
		}
   | LPARENT Exp %prec MISSING_RPARENT		
   		{
			lparent_num--;
   			msgFactory.newError(e_rparent, @2.last_line, @2.last_column);
			if (!errorFlag) {
				errorFlag = true;
				clearAstNodes();	
			}
   		}
   | Exp ERR_RPARENT 	
   		{
			lparent_num++;
   			msgFactory.newError(e_lparent, @1.first_line, @1.first_column);
			if (!errorFlag) {
				errorFlag = true;
				clearAstNodes();	
			}
   		}

   | Exp PLUS Exp 		
   		{
			if (!errorFlag) {
				$$ = new BinaryExpNode('+', (ExpNode*)$1, (ExpNode*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
   | Exp MINUS Exp 		
   		{
			if (!errorFlag) {
				$$ = new BinaryExpNode('-', (ExpNode*)$1, (ExpNode*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
   | Exp MULT Exp 		
   		{
			if (!errorFlag) {
				$$ = new BinaryExpNode('*', (ExpNode*)$1, (ExpNode*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
   | Exp DIV Exp 		
   		{
			if (!errorFlag) {
				$$ = new BinaryExpNode('/', (ExpNode*)$1, (ExpNode*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
   | Exp MOD Exp 		
   		{
			if (!errorFlag) {
				$$ = new BinaryExpNode('%', (ExpNode*)$1, (ExpNode*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}

   | Exp error  Exp 	
   		{
   			yyerrok; 
   			msgFactory.newError(e_miss_op, @2.first_line, @2.last_column - 1);
			if (!errorFlag) {
				errorFlag = true;
				clearAstNodes();
			}
		}

   | PLUS Exp %prec POS 
   		{
			if (!errorFlag) {
				$$ = new UnaryExpNode('+', (ExpNode*)$2);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
   | MINUS Exp %prec NEG 
   		{
			if (!errorFlag) {
			 	$$ = new UnaryExpNode('-', (ExpNode*)$2);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
   ;

ExpList: Exp 		
	   		{
				if (!errorFlag) {
					$$ = new NodeList($1);
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}
			}
	   | ExpList COMMA Exp 
	   		{
				if (!errorFlag) {
					$1->append($3);
					$$ = $1;
					$$->setLoc((Loc*)&(@$));
				}
			}
	   ;

Type: INTTYPE
	| FLOATTYPE
	| CHARTYPE
	;

Decl: ConstDecl 		
		{
			if (!errorFlag) {
				$$ = $1;
			}
		}
	| VarDecl 			
		{
			if (!errorFlag) {
				$$ = $1;
			}
		}
	;

ConstDecl: CONST Type ConstDefList SEMICOLON 
		 	{
				if (!errorFlag) {
					$$ = new ConstDeclNode($3);
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}
			}
		 | CONST ConstDefList SEMICOLON 
		 	{
				msgFactory.newWarning(w_miss_int, @2.first_line, @2.first_column);
				if (!errorFlag) {
					$$ = new ConstDeclNode($2);
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}
			}
		 ;

ConstDefList: ConstDef 	
				{
					if (!errorFlag) {
						$$ = new NodeList($1);
						$$->setLoc((Loc*)&(@$));
						astNodes.push_back($$);
					}
				}
			| ConstDefList COMMA ConstDef 
				{
					if (!errorFlag) {
						$1->append($3);
						$$ = $1;
						$$->setLoc((Loc*)&(@$));
					}
				}
			;

ConstDef: ID ASIGN Exp 
			{
				if (!errorFlag) {
					$$ = new IdConstDefNode($1, (ExpNode*)$3);
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}
				else {
					delete $1;
				}
			}
		| ID LBRACKET Exp RBRACKET ASIGN LBRACE ExpList RBRACE 
			{
				if (!errorFlag) {
					$$ = new ArrayConstDefNode($1, (ExpNode*)$3, $7);
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}
				else {
					delete $1;
				}
			}
		| ID LBRACKET RBRACKET ASIGN LBRACE ExpList RBRACE
			{
				if (!errorFlag) {
					$$ = new ArrayConstDefNode($1, NULL, $6);
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}
				else {
					delete $1;
				}
			}
   		| ID LBRACKET Exp RBRACKET 
			{
				yyerrok; 
   				msgFactory.newError(e_const_decl_not_init, @1.first_line, @1.last_column);
				delete $1;
				if (!errorFlag) {
					errorFlag = true;
					clearAstNodes();
				}
			}
		;


VarDecl: Type VarList SEMICOLON 
	   		{
				if (!errorFlag) {
					$$ = new VarDeclNode($2);
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}	
			}
	   ;

VarList: Var			
	   		{
				if (!errorFlag) {
					$$ = new NodeList($1);
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}
			}
	   | VarList COMMA Var 
	   		{
				if (!errorFlag) {
					$1->append($3);
					$$ = $1;
					$$->setLoc((Loc*)&(@$));
				}
			}
	   ;

Var: ID 				
   		{
			if (!errorFlag) {
				$$ = new IdVarDefNode($1, NULL);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
			else {
				delete $1;
			}
		}
   | ID LBRACKET Exp RBRACKET 
   		{
			if (!errorFlag) {
				$$ = new ArrayVarDefNode($1, (ExpNode*)$3, NULL);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
			else {
				delete $1;
			}
		}
   | ID ASIGN Exp 		
   		{
			if (!errorFlag) {
				$$ = new IdVarDefNode($1, (ExpNode*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
			else {
				delete $1;
			}
		}
   | ID LBRACKET Exp RBRACKET ASIGN LBRACE ExpList RBRACE 
   		{
			if (!errorFlag) {
				$$ = new ArrayVarDefNode($1, (ExpNode*)$3, $7);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
			else {
				delete $1;
			}
		}
   | ID LBRACKET RBRACKET ASIGN LBRACE ExpList RBRACE 
   		{
			if (!errorFlag) {
				$$ = new ArrayVarDefNode($1, NULL, $6);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
			else {
				delete $1;
			}
		}
   ;


ArgNameList: Type ID
	  	{
			if (!errorFlag) {
				$$ = new NodeList(new IdNode($2));
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
			else {
				delete $2;
			}
		}
	  | ArgNameList COMMA Type ID
	  	{
			if (!errorFlag) {
				$1->append(new IdNode($4));
				$$ = $1;
				$$->setLoc((Loc*)&(@$));
			}
			else {
				delete $4;
			}
		}
	  ;


FuncDecl: VOID ID LPARENT RPARENT SEMICOLON
			{
				if (!errorFlag) {
					$$ = new FuncDeclNode($2, NULL);
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}
				else {
					delete $2;
				}
			}
		| VOID ID LPARENT ArgNameList RPARENT SEMICOLON
			{
				if (!errorFlag) {
					$$ = new FuncDeclNode($2, (NodeList*)$4);
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}
				else {
					delete $2;
				}
			}
		;


ExternDecl: EXTERN FuncDecl
	   		{
				if (!errorFlag) {
					$$ = $2;
					$$->setLoc((Loc*)&(@$));
				}
			}
	   ;


FuncDef: VOID ID LPARENT RPARENT Block 	
	   		{
				if (!errorFlag) {
					FuncDeclNode *decl = new FuncDeclNode($2, NULL);
					decl->setLoc((Loc*)&(@$));
					$$ = new FuncDefNode(decl, (BlockNode*)$5);
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}
				else {
					delete $2;
				}
			}
		| VOID ID LPARENT ArgNameList RPARENT Block
			{
				if (!errorFlag) {
					FuncDeclNode *decl = new FuncDeclNode($2, (NodeList*)$4);
					decl->setLoc((Loc*)&(@$));
					$$ = new FuncDefNode(decl, (BlockNode*)$6);
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}
				else {
					delete $2;
				}
			}
		;

Block: LBRACE BlockItemList RBRACE 
	 	{
			if (!errorFlag) {
				$$ = new BlockNode($2);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
	 ;

BlockItemList: BlockItem 		
			 	{
					if (!errorFlag) {
						$$ = new NodeList($1);
						$$->setLoc((Loc*)&(@$));
						astNodes.push_back($$);
					}
				}
			 | BlockItemList BlockItem 
			 	{
					if (!errorFlag) {
						$1->append($2);
						$$ = $1;
						$$->setLoc((Loc*)&(@$));
					}
				}
			 ;

BlockItem: Decl 		
		 	{
				if (!errorFlag) {
					$$ = $1;
				}
			}
		 | Stmt 		
		 	{
				if (!errorFlag) {
					$$ = $1;
				}
			}
		 ;

Stmt: LVal ASIGN Exp SEMICOLON 
		{
			if (!errorFlag) {
				$$ = new AssignStmtNode((LValNode*)$1, (ExpNode*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}


	| ID LPARENT RPARENT SEMICOLON 
		{
			if (!errorFlag) {
				$$ = new FunCallStmtNode($1, NULL);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
			else {
				delete $1;
			}
		}

	| ID LPARENT ExpList RPARENT SEMICOLON
		{
			if (!errorFlag) {
				$$ = new FunCallStmtNode($1, $3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
			else {
				delete $1;
			}
		}

	| Block 			
		{
			if (!errorFlag) {
				$$ = new BlockStmtNode((BlockNode*)$1);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
	
	| IF LPARENT Cond RPARENT Stmt %prec NO_ELSE	
		{
			if (!errorFlag) {
				$$ = new IfStmtNode((CondNode*)$3, (StmtNode*)$5, NULL);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}

	| IF LPARENT Cond RPARENT Stmt ELSE Stmt  
		{
			if (!errorFlag) {
				$$ = new IfStmtNode((CondNode*)$3, (StmtNode*)$5, (StmtNode*)$7);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}

	| WHILE LPARENT Cond RPARENT Stmt 
		{
			if (!errorFlag) {
				$$ = new WhileStmtNode((CondNode*)$3, (StmtNode*)$5);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}

	| BREAK SEMICOLON
		{
			if (!errorFlag) {
				$$ = new BreakStmtNode();
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}

	| CONTINUE SEMICOLON
		{
			if (!errorFlag) {
				$$ = new ContinueStmtNode();
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}

	| SEMICOLON 		
		{
			if (!errorFlag) {
				$$ = new EmptyNode();
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
	;

Cond: LPARENT Cond RPARENT
		{
			if (!errorFlag) {
				$$ = $2;
				$$->setLoc((Loc*)&(@$));
			}
		}

	| Cond OR Cond
		{
			if (!errorFlag) {
				$$ = new CondNode(OR_OP, $1, $3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}

	| Cond AND Cond
		{
			if (!errorFlag) {
				$$ = new CondNode(AND_OP, $1, $3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}

	| NOT Cond 
		{
			if (!errorFlag) {
				$$ = new CondNode(NOT_OP, NULL, $2);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}

	| Exp LT Exp 		
		{
			if (!errorFlag) {
				$$ = new CondNode(LT_OP, $1, $3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}

	| Exp GT Exp 		
		{
			if (!errorFlag) {
				$$ = new CondNode(GT_OP, $1, $3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}

	| Exp LTE Exp 		
		{
			if (!errorFlag) {
				$$ = new CondNode(LTE_OP, $1, $3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}

	| Exp GTE Exp 		
		{
			if (!errorFlag) {
				$$ = new CondNode(GTE_OP, $1, $3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}

	| Exp EQ Exp 		
		{
			if (!errorFlag) {
				$$ = new CondNode(EQ_OP, $1, $3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}

	| Exp NEQ Exp 		
		{
			if (!errorFlag) {
				$$ = new CondNode(NEQ_OP, $1, $3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
	;


%%

int yyerror(const char *msg)
{
	printf("%s\n", msg);
	return 0;
}


void clearAstNodes()
{
	while (!astNodes.empty()) {
	 	delete astNodes.front();
		astNodes.pop_front();
	}
}


