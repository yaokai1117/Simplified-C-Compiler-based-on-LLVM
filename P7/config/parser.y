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
	int num;
	std::string *name;
	Node *node;
	NodeList *nodeList;
}


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

%type <num> NUM
%type <name> ID
%type <node> CompUnit FuncDef LVal Exp Decl ConstDecl VarDecl ConstDef Var 
%type <node> Block BlockItem Stmt Cond
%type <nodeList> ExpList ConstDefList VarList BlockItemList

%destructor {
	delete ($$);
} ID

%%

CompUnit: Decl 				
			{
				if (!errorFlag) {
					root = new CompUnitNode($1);
					root->setLoc((Loc*)&(@$));
					astNodes.push_back(root);
				}
			}
		| FuncDef 			
			{
				if (!errorFlag) {
					root = new CompUnitNode($1);
					root->setLoc((Loc*)&(@$));
					astNodes.push_back(root);
				}
			}
		| CompUnit Decl 	
			{
				if (!errorFlag) {
					root->append($2);
					root->setLoc((Loc*)&(@$));
				}
			}
		| CompUnit FuncDef 	
			{
				if (!errorFlag) {
					root->append($2);
					root->setLoc((Loc*)&(@$));
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

ConstDecl: CONST INT ConstDefList SEMICOLON 
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

VarDecl: INT VarList SEMICOLON 
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

FuncDef: VOID ID LPARENT RPARENT Block 	
	   		{
				if (!errorFlag) {
					$$ = new FuncDefNode($2, (BlockNode*)$5);
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
				$$ = new FunCallStmtNode($1);
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

	| SEMICOLON 		
		{
		}
	;

Cond: Exp LT Exp 		
		{
			if (!errorFlag) {
				$$ = new CondNode(LT_OP, (ExpNode*)$1, (ExpNode*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
	| Exp GT Exp 		
		{
			if (!errorFlag) {
				$$ = new CondNode(GT_OP, (ExpNode*)$1, (ExpNode*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
	| Exp LTE Exp 		
		{
			if (!errorFlag) {
				$$ = new CondNode(LTE_OP, (ExpNode*)$1, (ExpNode*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
	| Exp GTE Exp 		
		{
			if (!errorFlag) {
				$$ = new CondNode(GTE_OP, (ExpNode*)$1, (ExpNode*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
	| Exp EQ Exp 		
		{
			if (!errorFlag) {
				$$ = new CondNode(EQ_OP, (ExpNode*)$1, (ExpNode*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
	| Exp NEQ Exp 		
		{
			if (!errorFlag) {
				$$ = new CondNode(NEQ_OP, (ExpNode*)$1, (ExpNode*)$3);
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


