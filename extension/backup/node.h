#ifndef _NODE_H_
#define _NODE_H_

#include <stdio.h>
#include <string>
#include <list>
#include "dumpdot.h"

#include "llvm/IR/Verifier.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

using namespace std;


typedef enum {
	// exp types
	NUM_AST,
	FNUM_AST,
	CHAR_AST,
	ID_AST,
	ARRAY_ITEM_AST,
	BINARY_EXP_AST,
	UNARY_EXP_AST,

	// const definition types
	ID_CONST_DEF_AST,
	ARRAY_CONST_DEF_AST,
	// variable definition types
	ID_VAR_DEF_AST,
	ARRAY_VAR_DEF_AST,

	BLOCK_AST,
	CONST_DECL_AST,
	VAR_DECL_AST,

	ASSIGN_STMT_AST,
	FUNCALL_STMT_AST,
	BLOCK_STMT_AST,
	IF_STMT_AST,
	WHILE_STMT_AST,

	FUNC_DECL_AST,
	FUNC_DEF_AST,

	COND_AST

} NodeType;

typedef enum {
	NONE_TYPE,
	INT_TYPE,
	FLOAT_TYPE,
	CHAR_TYPE,
} ValueType;

typedef enum {
	LT_OP,
	GT_OP,
	LTE_OP,
	GTE_OP,
	EQ_OP,
	NEQ_OP,
	AND_OP,
	OR_OP,
	NOT_OP
} OpType;

typedef struct {
    int first_line;
    int first_column;
    int last_line;
    int last_column;
} Loc;


class Node {
public:
    Node();
	virtual ~Node();
    void setLoc(Loc* loc);
    virtual int dumpdot(DumpDOT *dumper) = 0;
	virtual llvm::Value *codegen() = 0;

    NodeType type;
    ValueType valTy;
    Loc* loc;
};

class NodeList : public Node{
public:
	NodeList(Node *node);
	~NodeList();
	void append(Node *node);
	void dumpdot(DumpDOT *dumper, int nRoot, int pos);
	int dumpdot(DumpDOT *dumper);
	virtual llvm::Value *codegen();

	list<Node*> nodes;
};


class ExpNode : public Node {
public:
    virtual int dumpdot(DumpDOT *dumper) = 0;
	virtual llvm::Value *codegen() = 0;
};


class NumNode : public ExpNode {
public:
    NumNode(int val);
	~NumNode();
    int dumpdot(DumpDOT *dumper);
	virtual llvm::Value *codegen();

    int val;
};


class FNumNode : public ExpNode {
public:
    FNumNode(double fval);
	~FNumNode();
    int dumpdot(DumpDOT *dumper);
	virtual llvm::Value *codegen() {return 0;}

    double fval;
};


class CharNode : public ExpNode {
public:
	CharNode(char cval);
	~CharNode();
	int dumpdot(DumpDOT *dumper);
	virtual llvm::Value *codegen() {return 0;}

	char cval;
};


class BinaryExpNode : public ExpNode {
public:
    BinaryExpNode(char op, ExpNode *lhs, ExpNode *rhs);
	~BinaryExpNode();
    int dumpdot(DumpDOT *dumper);
	virtual llvm::Value *codegen();

    char op;
    ExpNode *lhs, *rhs;
};


class UnaryExpNode : public ExpNode {
public:
    UnaryExpNode(char op, ExpNode *operand);
	~UnaryExpNode();
    int dumpdot(DumpDOT *dumper);
	virtual llvm::Value *codegen();

    char op;
    ExpNode *operand;
};


class LValNode : public ExpNode {
public:
    virtual int dumpdot(DumpDOT *dumper) = 0;
	virtual llvm::Value *codegen() = 0;
};


class IdNode : public LValNode {
public:
    IdNode(std::string* name);
	~IdNode();
    int dumpdot(DumpDOT *dumper);
	virtual llvm::Value *codegen();

    std::string *name;
};


class ArrayItemNode : public LValNode {
public:
	ArrayItemNode(std::string *name, ExpNode *index);
	~ArrayItemNode();
    int dumpdot(DumpDOT *dumper);
	virtual llvm::Value *codegen();

	std::string *name;
	ExpNode *index;
};


class ConstDefNode : public Node {
public:
    virtual int dumpdot(DumpDOT *dumper) = 0;
	virtual llvm::Value *codegen() = 0;
};


class IdConstDefNode : public ConstDefNode {
public:
	IdConstDefNode(std::string *name, ExpNode *value);
	~IdConstDefNode();
    int dumpdot(DumpDOT *dumper);
	virtual llvm::Value *codegen();

	std::string *name;
	ExpNode *value;
};


class ArrayConstDefNode : public ConstDefNode {
public:
	ArrayConstDefNode(std::string *name, ExpNode *size, NodeList *values);
	~ArrayConstDefNode();
    int dumpdot(DumpDOT *dumper);
	virtual llvm::Value *codegen();
	
	bool hasSize;
	std::string *name;
	ExpNode *size;
	NodeList *values;
};


class VarDefNode : public Node {
public:
    virtual int dumpdot(DumpDOT *dumper) = 0;
	virtual llvm::Value *codegen() = 0;
};


class IdVarDefNode : public VarDefNode {
public:
	IdVarDefNode(std::string *name, ExpNode *value);
	~IdVarDefNode();
    int dumpdot(DumpDOT *dumper);
	virtual llvm::Value *codegen();
	
	bool isAssigned;
	std::string *name;
	ExpNode *value;
};


class ArrayVarDefNode : public VarDefNode {
public:
	ArrayVarDefNode(std::string *name, ExpNode *size, NodeList *values);
	~ArrayVarDefNode();
    int dumpdot(DumpDOT *dumper);
	virtual llvm::Value *codegen();

	bool isAssigned;
	bool hasSize;
	std::string *name;
	ExpNode *size;
	NodeList *values;
};


class BlockItemNode : public Node {
public:
    virtual int dumpdot(DumpDOT *dumper) = 0;
	virtual llvm::Value *codegen() = 0;
};


class DeclNode : public BlockItemNode {
public:
    virtual int dumpdot(DumpDOT *dumper) = 0;
	virtual llvm::Value *codegen() = 0;
};


class StmtNode : public BlockItemNode{
public:
    virtual int dumpdot(DumpDOT *dumper) = 0;
	virtual llvm::Value *codegen() = 0;
};



class EmptyNode : public StmtNode {
public:
	EmptyNode();
	~EmptyNode();
	virtual int dumpdot(DumpDOT *dumper);
	virtual llvm::Value *codegen();
};


class BlockNode : public Node {
public:
	BlockNode(NodeList *blockItems);
	~BlockNode();
    int dumpdot(DumpDOT *dumper);
	virtual llvm::Value *codegen();

	NodeList *blockItems;
};


class ConstDeclNode : public DeclNode {
public:
	ConstDeclNode(NodeList *defList);
	~ConstDeclNode();
    int dumpdot(DumpDOT *dumper);
	virtual llvm::Value *codegen();

	NodeList *defList;
};


class VarDeclNode : public DeclNode {
public:
	VarDeclNode(NodeList *defList);
	~VarDeclNode();
    int dumpdot(DumpDOT *dumper);
	virtual llvm::Value *codegen();
	
	NodeList *defList;
};


class AssignStmtNode : public StmtNode {
public:
	AssignStmtNode(LValNode *lval, ExpNode *exp);
	~AssignStmtNode();
    int dumpdot(DumpDOT *dumper);
	virtual llvm::Value *codegen();
	
	LValNode *lval;
	ExpNode *exp;
};


class FunCallStmtNode : public StmtNode {
public:
	FunCallStmtNode(std::string *name, NodeList *argv);
	~FunCallStmtNode();
    int dumpdot(DumpDOT *dumper);
    llvm::Value *codegen();

    bool hasArgs;
    NodeList *argv;
	std::string *name;
};


class BlockStmtNode : public StmtNode {
public:
	BlockStmtNode(BlockNode *block);
	~BlockStmtNode();
    int dumpdot(DumpDOT *dumper);
	virtual llvm::Value *codegen();

	BlockNode *block;
};


class CondNode : public Node {
public:
	CondNode(OpType op, Node *lhs, Node *rhs);
	~CondNode();
    int dumpdot(DumpDOT *dumper);
	virtual llvm::Value *codegen();

	OpType op;
	Node *lhs;
	Node *rhs;
};


class IfStmtNode : public StmtNode {
public:
	IfStmtNode(CondNode *cond, StmtNode *then_stmt, StmtNode *else_stmt);
	~IfStmtNode();
    int dumpdot(DumpDOT *dumper);
	virtual llvm::Value *codegen();

	bool hasElse;
	CondNode *cond;
	StmtNode *then_stmt;
	StmtNode *else_stmt;
};


class WhileStmtNode : public StmtNode {
public:
	WhileStmtNode(CondNode *cond, StmtNode *do_stmt);
	~WhileStmtNode();
    int dumpdot(DumpDOT *dumper);
	virtual llvm::Value *codegen();

	CondNode *cond;
	StmtNode *do_stmt;
};


class BreakStmtNode : public StmtNode {
public:
	BreakStmtNode();
	~BreakStmtNode();
	int dumpdot(DumpDOT *dumper);
	virtual llvm::Value *codegen();
};


class ContinueStmtNode : public StmtNode {
public:
	ContinueStmtNode();
	~ContinueStmtNode();
	int dumpdot(DumpDOT *dumper);
	virtual llvm::Value *codegen();
};


class FuncDeclNode : public Node {
public:
	FuncDeclNode(std::string *name, NodeList *argv);
	~FuncDeclNode();
	int dumpdot(DumpDOT *dumper);
	virtual llvm::Function *codegen();

	bool hasArgs;
	std::string *name;
	NodeList *argv;
};


class FuncDefNode : public Node {
public:
	FuncDefNode(FuncDeclNode *decl, BlockNode *block);
	~FuncDefNode();
    int dumpdot(DumpDOT *dumper);
	virtual llvm::Function *codegen();

	FuncDeclNode *decl;
	BlockNode *block;
};

class CompUnitNode : public Node {
public:
	CompUnitNode(Node *node);
	~CompUnitNode();
    int dumpdot(DumpDOT *dumper);
	void append(Node *node);
	virtual llvm::Value *codegen();

	list<Node*> nodes;
};

#endif
