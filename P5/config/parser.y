%{
#include <cstdio>
#include <string>
#include <list>
#define DEBUG
#include "util.h"
#include "global.h"
#include "msgfactory.h"
#include "node.h"

extern int yylex();
extern int yyerror(const char *msg);
extern int lparent_num;

MsgFactory msgFactory;
CompUnitNode *root;

bool errorFlag = false;
list<Node*> astNodes;
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

%%

CompUnit: Decl 				
			{
				debug("CompUnit ::= Decl\n");
				if (!errorFlag) {
					root = new CompUnitNode($1);
					astNodes.push_back(root);
				}
			}
		| FuncDef 			
			{
				debug("CompUnit ::= FuncDef\n");
				if (!errorFlag) {
					root = new CompUnitNode($1);
					astNodes.push_back(root);
				}
			}
		| CompUnit Decl 	
			{
				debug("CompUnit ::= CompUnit Decl\n");
				if (!errorFlag) 
					root->append($2);
			}
		| CompUnit FuncDef 	
			{
				debug("CompUnit ::= CompUnit FuncDef\n");
				if (!errorFlag)
					root->append($2);
			}
		;

		
LVal: ID 				
		{
			debug("LVal ::= ID\n");
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
			debug("LVal ::= ID '[' Exp ']'\n");
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
			debug("Exp ::= LVal\n");
			if (!errorFlag) {
				$$ = $1;
			}
		}
   | NUM 				
   		{
			debug("Exp ::= NUM\n");
			if (!errorFlag) {
				$$ = new NumNode($1);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}

   | LPARENT Exp RPARENT 
   		{
			debug("Exp ::= '(' Exp ')'\n");
			if (!errorFlag) {
				$$ = $2;
			}
		}
   | LPARENT Exp %prec MISSING_RPARENT		
   		{
			lparent_num--;
   			Error err = msgFactory.newError(e_rparent, @2.last_line, @2.last_column);
			msgFactory.showMsg(&err);
			debug("Exp ::= '(' Exp ')' 		[error recovery]\n");
			if (!errorFlag) {
				errorFlag = true;
				clearAstNodes();	
			}
   		}
   | Exp ERR_RPARENT 	
   		{
			lparent_num++;
   			Error err = msgFactory.newError(e_lparent, @1.first_line, @1.first_column);
			msgFactory.showMsg(&err);
			debug("Exp ::= '(' Exp ')' 		[error recovery]\n");
			if (!errorFlag) {
				errorFlag = true;
				clearAstNodes();	
			}
   		}

   | Exp PLUS Exp 		
   		{
			debug("Exp ::= Exp PLUS Exp\n");
			if (!errorFlag) {
				$$ = new BinaryExpNode('+', (ExpNode*)$1, (ExpNode*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
   | Exp MINUS Exp 		
   		{
			debug("Exp ::= Exp MINUS Exp\n");
			if (!errorFlag) {
				$$ = new BinaryExpNode('-', (ExpNode*)$1, (ExpNode*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
   | Exp MULT Exp 		
   		{
			debug("Exp ::= Exp MULT Exp\n");
			if (!errorFlag) {
				$$ = new BinaryExpNode('*', (ExpNode*)$1, (ExpNode*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
   | Exp DIV Exp 		
   		{
			debug("Exp ::= Exp DIV Exp\n");
			if (!errorFlag) {
				$$ = new BinaryExpNode('/', (ExpNode*)$1, (ExpNode*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
   | Exp MOD Exp 		
   		{
			debug("Exp ::= Exp MOD Exp\n");
			if (!errorFlag) {
				$$ = new BinaryExpNode('%', (ExpNode*)$1, (ExpNode*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}

   | Exp error  Exp 	
   		{
   			yyerrok; 
   			Error err = msgFactory.newError(e_miss_op, @2.first_line, @2.last_column - 1);
			msgFactory.showMsg(&err);
			debug("Exp ::= Exp Exp 		[error recovery]\n");
			if (!errorFlag) {
				errorFlag = true;
				clearAstNodes();
			}
		}

   | PLUS Exp %prec POS 
   		{
			debug("Exp ::= POS Exp\n");
			if (!errorFlag) {
				$$ = new UnaryExpNode('+', (ExpNode*)$2);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
   | MINUS Exp %prec NEG 
   		{
			debug("Exp ::= NEG Exp\n");
			if (!errorFlag) {
			 	$$ = new UnaryExpNode('-', (ExpNode*)$2);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
   ;

ExpList: Exp 		
	   		{
				debug("ExpList ::= Exp\n");
				if (!errorFlag) {
					$$ = new NodeList($1);
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}
			}
	   | ExpList COMMA Exp 
	   		{
				debug("ExpList ::= ExpList ',' Exp\n");
				if (!errorFlag) {
					$1->append($3);
					$$ = $1;
				}
			}
	   ;

Decl: ConstDecl 		
		{
			debug("Decl ::= ConstDecl\n");
			if (!errorFlag) {
				$$ = $1;
			}
		}
	| VarDecl 			
		{
			debug("Decl ::= VarDecl\n");
			if (!errorFlag) {
				$$ = $1;
			}
		}
	;

ConstDecl: CONST INT ConstDefList SEMICOLON 
		 	{
				debug("ConstDecl ::= CONST INT ConstDefList ';'\n");
				if (!errorFlag) {
					$$ = new ConstDeclNode($3);
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}
			}
		 | CONST ConstDefList SEMICOLON 
		 	{
				Warning warn = msgFactory.newWarning(w_miss_int, @2.first_line, @2.first_column);
				msgFactory.showMsg(&warn);
				debug("ConstDecl ::= CONST INT ConstDefList ';' 	[error recovery]\n");
				if (!errorFlag) {
					$$ = new ConstDeclNode($2);
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}
			}
		 ;

ConstDefList: ConstDef 	
				{
					debug("ConstDefList ::= ConstDef\n");
					if (!errorFlag) {
						$$ = new NodeList($1);
						$$->setLoc((Loc*)&(@$));
						astNodes.push_back($$);
					}
				}
			| ConstDefList COMMA ConstDef 
				{
					debug("ConstDefList ::= ConstDefList ',' ConstDef\n");
					if (!errorFlag) {
						$1->append($3);
						$$ = $1;
					}
				}
			;

ConstDef: ID ASIGN Exp 
			{
				debug("ConstDef ::= ID '=' Exp\n");
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
				debug("ConstDef ::= ID '[' Exp ']' '=' '{' ExpList '}'\n");
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
				debug("ConstDef ::= ID '[' ']' '=' '{' ExpList '}'\n");
				if (!errorFlag) {
					$$ = new ArrayConstDefNode($1, NULL, $6);
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}
				else {
					delete $1;
				}
			}
		;

VarDecl: INT VarList SEMICOLON 
	   		{
				debug("VarDecl ::= INT VarList ';'\n");
				if (!errorFlag) {
					$$ = new VarDeclNode($2);
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}	
			}
	   ;

VarList: Var			
	   		{
				debug("VarList ::= Var\n");
				if (!errorFlag) {
					$$ = new NodeList($1);
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}
			}
	   | VarList COMMA Var 
	   		{
				debug("VarList ::= VarList ',' Var\n");
				if (!errorFlag) {
					$1->append($3);
					$$ = $1;
				}
			}
	   ;

Var: ID 				
   		{
			debug("Var ::= ID\n");
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
			debug("Var ::= ID '[' Exp ']'\n");
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
			debug("Var ::= ID '=' Exp\n");
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
			debug("Var ::= ID '[' Exp ']' '=' '{' ExpList '}'\n");
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
			debug("Var ::= ID '[' ']' '=' '{' ExpList '}'\n");
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
				debug("FuncDef ::= VOID ID '(' ')' Block\n");
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
			debug("Block ::= '{' BlockItemList '}'\n");
			if (!errorFlag) {
				$$ = new BlockNode($2);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
	 ;

BlockItemList: BlockItem 		
			 	{
					debug("BlockItemList ::= BlockItem\n");
					if (!errorFlag) {
						$$ = new NodeList($1);
						$$->setLoc((Loc*)&(@$));
						astNodes.push_back($$);
					}
				}
			 | BlockItemList BlockItem 
			 	{
					debug("BlockItemList ::= BlockItemList BlockItem\n");
					if (!errorFlag) {
						$1->append($2);
						$$ = $1;
					}
				}
			 ;

BlockItem: Decl 		
		 	{
				debug("BlockItem ::= Decl\n");
				if (!errorFlag) {
					$$ = $1;
				}
			}
		 | Stmt 		
		 	{
				debug("BlockItem ::= Stmt\n");
				if (!errorFlag) {
					$$ = $1;
				}
			}
		 ;

Stmt: LVal ASIGN Exp SEMICOLON 
		{
			debug("Stmt ::= LVal '=' Exp ';'\n");
			if (!errorFlag) {
				$$ = new AssignStmtNode((LValNode*)$1, (ExpNode*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}


	| ID LPARENT RPARENT SEMICOLON 
		{
			debug("Stmt ::= ID '(' ')' ';'\n");
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
			debug("Stmt ::= Block\n");
			if (!errorFlag) {
				$$ = new BlockStmtNode((BlockNode*)$1);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
	
	| IF LPARENT Cond RPARENT Stmt %prec NO_ELSE	
		{
			debug("Stmt ::= IF '(' Cond ')' Stmt\n");
			if (!errorFlag) {
				$$ = new IfStmtNode((CondNode*)$3, (StmtNode*)$5, NULL);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}

	| IF LPARENT Cond RPARENT Stmt ELSE Stmt  
		{
			debug("Stmt ::= IF '(' Cond ')' Stmt ELSE Stmt\n");
			if (!errorFlag) {
				$$ = new IfStmtNode((CondNode*)$3, (StmtNode*)$5, (StmtNode*)$7);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}

	| WHILE LPARENT Cond RPARENT Stmt 
		{
			debug("Stmt ::= WHILE '(' Cond ')' Stmt\n");
			if (!errorFlag) {
				$$ = new WhileStmtNode((CondNode*)$3, (StmtNode*)$5);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}

	| SEMICOLON 		
		{
			debug("Stmt ::= ';'\n");
		}
	;

Cond: Exp LT Exp 		
		{
			debug("Cond ::= Exp LT Exp\n");
			if (!errorFlag) {
				$$ = new CondNode(LT_OP, (ExpNode*)$1, (ExpNode*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
	| Exp GT Exp 		
		{
			debug("Cond ::= Exp GT Exp\n");
			if (!errorFlag) {
				$$ = new CondNode(GT_OP, (ExpNode*)$1, (ExpNode*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
	| Exp LTE Exp 		
		{
			debug("Cond ::= Exp LTE Exp\n");
			if (!errorFlag) {
				$$ = new CondNode(LTE_OP, (ExpNode*)$1, (ExpNode*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
	| Exp GTE Exp 		
		{
			debug("Cond ::= Exp GTE Exp\n");
			if (!errorFlag) {
				$$ = new CondNode(GTE_OP, (ExpNode*)$1, (ExpNode*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
	| Exp EQ Exp 		
		{
			debug("Cond ::= Exp EQ Exp\n");
			if (!errorFlag) {
				$$ = new CondNode(EQ_OP, (ExpNode*)$1, (ExpNode*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
	| Exp NEQ Exp 		
		{
			debug("Cond ::= Exp NEQ Exp\n");
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


