%{
#include <cstdio>
#include <string>
#include <list>
#include <map>
#include "util.h"
#include "global.h"
#include "msgfactory.h"
#include "node.h"

// debug
#define YYDEBUG 1

extern int yylex();
extern int yyerror(const char *msg);

extern MsgFactory msgFactory;

extern CompUnitNode *root;
extern bool errorFlag;
extern list<Node*> astNodes;

void clearAstNodes();

static void insertType(ValueTypeS *pType, ValueTypeS *thisTy);
static void setAtomType(ValueTypeS *pType, ValueTypeS atomTy);

%}

%debug
%expect 1

%locations
%initial-action 
{
    msgFactory.initial(infile_name);	
	yydebug = 0;
};

%union
{
	int ival;
	double fval;
	char cval;
	std::string *name;
	Node *node;
	NodeList *nodeList;
	ValueTypeS vType;
	struct {
		std::string *name;
		ValueTypeS vType;
	} var;
}


%token CONST INTTYPE FLOATTYPE CHARTYPE EXTERN STATIC
%token IF ELSE WHILE VOID ID NUM FNUM CHAR RETURN BREAK CONTINUE STRUCT 
%token ASIGN LBRACE RBRACE LBRACKET RBRACKET LPARENT RPARENT 
%token COMMA SEMICOLON  

%precedence NO_ELSE
%precedence ELSE


%left OR
%left AND
%left NOT
%left EQ NEQ
%left LT GT LTE GTE
%left PLUS MINUS
%left MULT DIV MOD SINGLE_AND

%precedence NEG POS

%precedence REF DEREF

%precedence NO_BRACKET

%precedence RPARENT LPARENT LBRACKET RBRACKET DOT ARROW


%type <ival> NUM
%type <fval> FNUM
%type <cval> CHAR
%type <name> ID
%type <node> CompUnit CompUnitItem FuncDef FunCall LVal Exp 
%type <node> ExternDecl StaticDecl VarDecl VarDef AssignedVar StructDef
%type <node> Block BlockItem Stmt Cond 
%type <nodeList> ExpList VarList BlockItemList ArgNameList ArraySuffix
%type <vType> Type
%type <var> Var

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

CompUnitItem: VarDecl 				
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
		| StaticDecl
			{
				if (!errorFlag) {
					$$ = $1;
				}
			}
		| StructDef
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
	| Exp ArraySuffix
		{
			if (!errorFlag) {
				$$ = new ArrayItemNode((ExpNode*)$1, $2);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
	| MULT Exp %prec REF
		{
			if (!errorFlag) {
			 	$$ = new UnaryExpNode('*', (ExpNode*)$2);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}

	| Exp DOT ID
		{
			if (!errorFlag) {
				$$ = new StructItemNode((ExpNode*)$1, $3, false); 
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
	| Exp ARROW ID
		{
			if (!errorFlag) {
				$$ = new StructItemNode((ExpNode*)$1, $3, true); 
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
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

   | FunCall 
   		{
			if (!errorFlag) {
				$$ = $1;
				$$->setLoc((Loc*)&(@$));
			}
		}
   | LPARENT Exp RPARENT 
   		{
			if (!errorFlag) {
				$$ = $2;
				$$->setLoc((Loc*)&(@$));
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
   | SINGLE_AND Exp %prec DEREF
		{
			if (!errorFlag) {
			 	$$ = new UnaryExpNode('&', (ExpNode*)$2);
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
		{
			$$.type = INT_TYPE;
		}
	| FLOATTYPE
		{
			$$.type = FLOAT_TYPE;
		}
	| CHARTYPE
		{
			$$.type = CHAR_TYPE;
		}
	| VOID
		{
			$$.type = VOID_TYPE;
		}
	| STRUCT ID
		{
			if (!errorFlag) {
				$$.type = STRUCT_TYPE;
				$$.structName = $2;
			}
			else {
				delete $2;
			}
		}
	| CONST Type
		{
			$2.isConstant = true;
			$$ = $2;
		}
	;


ExternDecl: EXTERN VarDecl
			{
				if (!errorFlag) {
					std::list<Node *> &nodes = ((VarDeclNode*)$2)->defList->nodes;
					for (std::list<Node*>::iterator it = nodes.begin();
							it != nodes.end(); it++)  {
						(*it)->valueTy.isExtern = true;
					}

					$$ = $2;
					$$->setLoc((Loc*)&(@$));
				}
			}
	   ;


StaticDecl: STATIC VarDecl
		  	{
				if (!errorFlag) {
					std::list<Node *> &nodes = ((VarDeclNode*)$2)->defList->nodes;
					for (std::list<Node*>::iterator it = nodes.begin();
							it != nodes.end(); it++)  {
						(*it)->valueTy.isStatic = true;
					}

					$$ = $2;
					$$->setLoc((Loc*)&(@$));
				}
			}
		  ;


VarDecl: Type VarList SEMICOLON 
	   		{
				if (!errorFlag) {
					for (std::list<Node*>::iterator it = ($2)->nodes.begin();
							it != ($2)->nodes.end(); it++) {
						setAtomType(&((*it)->valueTy), $1);
					}
				
					$$ = new VarDeclNode($2);
					$$->valueTy = $1;
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}	
			}
	   ;

VarList: VarDef			
	   		{
				if (!errorFlag) {
					$$ = new NodeList($1);
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}
			}
	   | VarList COMMA VarDef
	   		{
				if (!errorFlag) {
					$1->append($3);
					$$ = $1;
					$$->setLoc((Loc*)&(@$));
				}
			}
	   ;


VarDef: Var
	  	{
			if (!errorFlag) {
				if ($1.vType.type == ARRAY_TYPE) {
					$$ = new ArrayVarDefNode($1.name, NULL);
					$$->valueTy = $1.vType;
					$$->valueTy.dim = $$->valueTy.argv->nodes.size();
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);

				}
				else if ($1.vType.type == FUNC_TYPE) {
					$$ = new FuncDeclNode($1.name, $1.vType.argv != NULL);
					$$->valueTy = $1.vType;
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}
				else {
					$$ = new IdVarDefNode($1.name, NULL);
					$$->valueTy = $1.vType;
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);

				}
			}
		}
	  | AssignedVar
	  	{
			$$ = $1;
		}
	  ;


AssignedVar: Var ASIGN Exp
		   	{
				if (!errorFlag) {
					if ($1.vType.type == FUNC_TYPE) 
						$$ = new FuncDeclNode($1.name, $1.vType.argv == NULL);
					else
						$$ = new IdVarDefNode($1.name, (ExpNode*)$3);

					$$->valueTy = $1.vType;
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}
			}
		   | Var ASIGN LBRACE ExpList RBRACE
			{
				if (!errorFlag) {
					$$ = new ArrayVarDefNode($1.name, $4);
					$$->valueTy = $1.vType;
					$$->valueTy.dim = $$->valueTy.argv->nodes.size();
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}
			}
		   ;


Var: ID
		{
			if (!errorFlag) {
				$$.name = $1;
				$$.vType = (ValueTypeS){ATOM_TYPE, 		// type
							NO_TYPE, 		// dstType
							false, 			// isConstant
							false, 			// isExtern
							false, 			// isStatic
							0, 				// dim
							NULL, 			// bases
							NULL, 			// argv
							NULL, 			// structName
							NULL, 			// atom
							false, 			// isComputed
							0}; 			// constVal
			}
			else {
				delete $1;
			}
		}

	| MULT Var
	 	{
			if (!errorFlag) {
				$$ = $2;
				ValueTypeS *thisTy = new ValueTypeS;
				*thisTy = (ValueTypeS){PTR_TYPE, 		// type
							NO_TYPE, 		// dstType
							false, 				// isConstant
							false, 				// isExtern
							false, 				// isStatic
							0,  				// dim
							NULL, 				// bases
							NULL, 				// argv
							NULL, 				// structName
							NULL, 				// atom
							false, 			// isComputed
							0}; 			// constVal
				insertType(&($$.vType), thisTy);
			}
		}

	| Var ArraySuffix %prec NO_BRACKET
		{
			if (!errorFlag) {
				$$ = $1;
				ValueTypeS *thisTy = new ValueTypeS;
				*thisTy = (ValueTypeS){ARRAY_TYPE,  		// type
							NO_TYPE, 		// dstType
							false, 				// isConstant
							false, 				// isExtern
							false, 				// isStatic
							$2->nodes.size(), 	// dim
							NULL, 				// bases
							(NodeList*)$2, 		// argv   
							NULL, 				// structName
							NULL,				// atom
							false, 			// isComputed
							0}; 			// constVal
				insertType(&($$.vType), thisTy);
			}
		}

	| Var LPARENT RPARENT  
	   	{
			if (!errorFlag) {
				$$ = $1;
				ValueTypeS *thisTy = new ValueTypeS;
				*thisTy = (ValueTypeS){FUNC_TYPE,  		// type
							NO_TYPE, 		// dstType
							false, 				// isConstant
							false, 				// isExtern
							false, 				// isStatic
							0, 				 	// dim
							NULL, 		 		// bases   
							NULL, 				// argv
							NULL, 				// structName
							NULL,				// atom
							false, 			// isComputed
							0}; 			// constVal

				insertType(&($$.vType), thisTy);
			}
		}

	| Var LPARENT ArgNameList RPARENT 
		{
			if (!errorFlag) {
				$$ = $1;
				ValueTypeS *thisTy = new ValueTypeS;
				*thisTy = (ValueTypeS){FUNC_TYPE,  		// type
							NO_TYPE, 		// dstType
							false, 				// isConstant
							false, 				// isExtern
							false, 				// isStatic
							0, 				 	// dim
							NULL, 		 		// bases   
							$3, 				// argv
							NULL, 				// structName
							NULL,				// atom
							false, 			// isComputed
							0}; 			// constVal
				insertType(&($$.vType), thisTy);
			}
		}

	| LPARENT Var RPARENT
		{
			if (!errorFlag) {
				$$ = $2;
			}
		}
	;


ArraySuffix: LBRACKET Exp RBRACKET
		   	{
				if (!errorFlag) {
					$$ = new NodeList($2);
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}
			}
		   | LBRACKET RBRACKET
		   	{
				if (!errorFlag) {
					$$ = new NodeList(NULL);
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}
			}
		   | ArraySuffix LBRACKET Exp RBRACKET
		   	{
				if (!errorFlag) {
					$1->append($3);
					$$ = $1;
					$$->setLoc((Loc*)&(@$));
				}
			}
		   ;


ArgNameList: Type Var 
	  	{
			if (!errorFlag) {
				IdNode *node = new IdNode($2.name);
				node->valueTy = $2.vType;
				setAtomType(&(node->valueTy), $1);
				$$ = new NodeList(node);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}
	  | ArgNameList COMMA Type Var 
	  	{
			if (!errorFlag) {
				IdNode *node = new IdNode($4.name);
				node->valueTy = $4.vType;
				setAtomType(&(node->valueTy), $3);
				$1->append(node);
				$$ = $1;
				$$->setLoc((Loc*)&(@$));
			}
		}
	  ;


FuncDef: Type Var Block 	
	   		{
				if (!errorFlag) {
					if ($2.vType.type != FUNC_TYPE) {
						errorFlag = true;
						yyerror("nodt func type\n");
					}

					FuncDeclNode *decl = new FuncDeclNode($2.name, $2.vType.argv != NULL);
					decl->valueTy = $2.vType;
					setAtomType(&(decl->valueTy), $1);

					$$ = new FuncDefNode(decl, (BlockNode*)$3);
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}
			}
		;


StructDef: STRUCT ID Block SEMICOLON
		 	{
				if (!errorFlag) {
					$$ = new StructDefNode($2, ((BlockNode*)$3)->blockItems);
					$$->setLoc((Loc*)&(@$));
					astNodes.push_back($$);
				}
				else {
					delete $2;
				}
			}
		 ;


FunCall: Exp LPARENT RPARENT
	   	{
			if (!errorFlag) {
				$$ = new FunCallNode((ExpNode*)$1, NULL);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
			else {
				delete $1;
			}
		}
	   | Exp LPARENT ExpList RPARENT
	   	{
			if (!errorFlag) {
				$$ = new FunCallNode((ExpNode*)$1, (NodeList*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
			else {
				delete $1;
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

BlockItem: VarDecl 		
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
				$$ = new AssignStmtNode((ExpNode*)$1, (ExpNode*)$3);
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
			}
		}

	| FunCall SEMICOLON
		{
			if (!errorFlag) {
				$$ = new FunCallStmtNode((FunCallNode*)($1));	
				$$->setLoc((Loc*)&(@$));
				astNodes.push_back($$);
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

	| RETURN Exp SEMICOLON
		{
			if (!errorFlag) {
				$$ = new ReturnStmtNode((ExpNode*)$2);
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

static void insertType(ValueTypeS *pType, ValueTypeS *thisTy)
{	
	ValueTypeS *pre = pType;
	while (pType->type != ATOM_TYPE) {
		pre = pType;
		pType = pType->atom;
	}

	// pTyte itself is an atom type
	if (pType == pre) {
		ValueTypeS tmp = *pType;
		*pType = *thisTy;
		pType->atom = thisTy;
		*thisTy = tmp;
		return;
	}

	// do insertion
	pre->atom = thisTy;
	thisTy->atom = pType;
}

static void setAtomType(ValueTypeS *pType, ValueTypeS atomTy)
{
	while (pType->type != ATOM_TYPE)
		pType = pType->atom;
	pType->type = atomTy.type;
	pType->isConstant = atomTy.isConstant;
	pType->isExtern = atomTy.isExtern;
	pType->isStatic = atomTy.isStatic;
	pType->structName = atomTy.structName;
}



