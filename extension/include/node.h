#ifndef _NODE_H_
#define _NODE_H_

#include <string>
#include <list>

class NodeList;
class Visitor;


using namespace std;


typedef enum {
	// exp types
	NUM_AST,
	FNUM_AST,
	CHAR_AST,
	ID_AST,
	ARRAY_ITEM_AST,
	STRUCT_ITEM_AST,
	BINARY_EXP_AST,
	UNARY_EXP_AST,
	FUN_CALL_AST,

	// variable definition types
	ID_VAR_DEF_AST,
	ARRAY_VAR_DEF_AST,

	BLOCK_AST,
	CONST_DECL_AST,
	VAR_DECL_AST,

	STRUCT_DEF_AST,

	ASSIGN_STMT_AST,
	FUNCALL_STMT_AST,
	BLOCK_STMT_AST,
	IF_STMT_AST,
	WHILE_STMT_AST,
	RETURN_STMT_AST,

	FUNC_DECL_AST,
	FUNC_DEF_AST,

	COND_AST

} NodeType;


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

typedef enum {
	INT_TYPE,
	FLOAT_TYPE,
	CHAR_TYPE,
	VOID_TYPE,
	STRUCT_TYPE,

	ATOM_TYPE,
	PTR_TYPE,
	ARRAY_TYPE,
	FUNC_TYPE,
	NO_TYPE
} ValueType;

typedef union {
	int ival;
	double fval;
	char cval;
} ConstVal;

typedef struct ValueTypeStuct{
	ValueType type;
	ValueType dstType;
	bool isConstant;
	bool isExtern;
	bool isStatic;
	int dim;
	int *base;
	NodeList *argv;
	std::string *structName;
	struct ValueTypeStuct *atom;
	bool isComputed;
	ConstVal constVal;
} ValueTypeS;


class Node {
public:
    Node();
	virtual ~Node();
    void setLoc(Loc* loc);
	virtual void accept(Visitor &visitor) = 0;

	ValueTypeS valueTy;
    NodeType type;
    Loc* loc;
};

class NodeList : public Node{
public:
	NodeList(Node *node);
	NodeList();
	~NodeList();
	void append(Node *node);
	virtual void accept(Visitor &visitor);

	list<Node*> nodes;
};


class ExpNode : public Node {
public:
	virtual void accept(Visitor &visitor) = 0;
};


class NumNode : public ExpNode {
public:
    NumNode(int val);
	~NumNode();
	virtual void accept(Visitor &visitor);

    int val;
};


class FNumNode : public ExpNode {
public:
    FNumNode(double fval);
	~FNumNode();
	virtual void accept(Visitor &visitor);

    double fval;
};


class CharNode : public ExpNode {
public:
	CharNode(char cval);
	~CharNode();
	virtual void accept(Visitor &visitor);

	char cval;
};


class BinaryExpNode : public ExpNode {
public:
    BinaryExpNode(char op, ExpNode *lhs, ExpNode *rhs);
	~BinaryExpNode();
	virtual void accept(Visitor &visitor);

    char op;
    ExpNode *lhs, *rhs;
};


class UnaryExpNode : public ExpNode {
public:
    UnaryExpNode(char op, ExpNode *operand);
	~UnaryExpNode();
	virtual void accept(Visitor &visitor);

    char op;
    ExpNode *operand;
};



class IdNode : public ExpNode {
public:
    IdNode(std::string* name);
	~IdNode();
	virtual void accept(Visitor &visitor);

    std::string *name;
};


class ArrayItemNode : public ExpNode {
public:
	ArrayItemNode(ExpNode *array, NodeList *index);
	~ArrayItemNode();
	virtual void accept(Visitor &visitor);

	ExpNode *array;
	NodeList *index;
};


class StructItemNode : public ExpNode {
public:
	StructItemNode(ExpNode *stru, std::string *itemName, bool isPointer);
	~StructItemNode();
	virtual void accept(Visitor &visitor);

	ExpNode *stru;
	std::string *itemName;
	bool isPointer;
};


class FunCallNode : public ExpNode {
public:
	FunCallNode(ExpNode *func, NodeList *argv);
	~FunCallNode();
	virtual void accept(Visitor &visitor);

    bool hasArgs;
    NodeList *argv;
    ExpNode *func;
};


class VarDefNode : public Node {
public:
	virtual void accept(Visitor &visitor) = 0;

	bool isAssigned;
	std::string *name;
};


class IdVarDefNode : public VarDefNode {
public:
	IdVarDefNode(std::string *name, ExpNode *value);
	~IdVarDefNode();
	virtual void accept(Visitor &visitor);
	
	ExpNode *value;
};


class ArrayVarDefNode : public VarDefNode {
public:
	ArrayVarDefNode(std::string *name, NodeList *values);
	~ArrayVarDefNode();
	virtual void accept(Visitor &visitor);

	NodeList *values;
};


class BlockItemNode : public Node {
public:
	virtual void accept(Visitor &visitor) = 0;
};


class DeclNode : public BlockItemNode {
public:
	virtual void accept(Visitor &visitor) = 0;
};


class StmtNode : public BlockItemNode{
public:
	virtual void accept(Visitor &visitor) = 0;
};



class EmptyNode : public StmtNode {
public:
	EmptyNode();
	~EmptyNode();
	virtual void accept(Visitor &visitor);
};


class BlockNode : public Node {
public:
	BlockNode(NodeList *blockItems);
	~BlockNode();
	virtual void accept(Visitor &visitor);

	NodeList *blockItems;
};


class VarDeclNode : public DeclNode {
public:
	VarDeclNode(NodeList *defList);
	~VarDeclNode();
	virtual void accept(Visitor &visitor);
	
	NodeList *defList;
};


class AssignStmtNode : public StmtNode {
public:
	AssignStmtNode(ExpNode *lval, ExpNode *exp);
	~AssignStmtNode();
	virtual void accept(Visitor &visitor);
	
	ExpNode *lval;
	ExpNode *exp;
};


class FunCallStmtNode : public StmtNode {
public:
	FunCallStmtNode(FunCallNode *funCall);
	~FunCallStmtNode();
	virtual void accept(Visitor &visitor);

	FunCallNode *funCall;
};


class BlockStmtNode : public StmtNode {
public:
	BlockStmtNode(BlockNode *block);
	~BlockStmtNode();
	virtual void accept(Visitor &visitor);

	BlockNode *block;
};


class CondNode : public Node {
public:
	CondNode(OpType op, Node *lhs, Node *rhs);
	~CondNode();
	virtual void accept(Visitor &visitor);

	OpType op;
	Node *lhs;
	Node *rhs;
};


class IfStmtNode : public StmtNode {
public:
	IfStmtNode(CondNode *cond, StmtNode *then_stmt, StmtNode *else_stmt);
	~IfStmtNode();
	virtual void accept(Visitor &visitor);

	bool hasElse;
	CondNode *cond;
	StmtNode *then_stmt;
	StmtNode *else_stmt;
};


class WhileStmtNode : public StmtNode {
public:
	WhileStmtNode(CondNode *cond, StmtNode *do_stmt);
	~WhileStmtNode();
	virtual void accept(Visitor &visitor);

	CondNode *cond;
	StmtNode *do_stmt;
};


class ReturnStmtNode : public StmtNode {
public:
	ReturnStmtNode(ExpNode *exp);
	~ReturnStmtNode();
	virtual void accept(Visitor &visitor);

	ExpNode *exp;
};


class BreakStmtNode : public StmtNode {
public:
	BreakStmtNode();
	~BreakStmtNode();
	virtual void accept(Visitor &visitor);
};


class ContinueStmtNode : public StmtNode {
public:
	ContinueStmtNode();
	~ContinueStmtNode();
	virtual void accept(Visitor &visitor);
};


class FuncDeclNode : public Node {
public:
	FuncDeclNode(std::string *name, bool hasArgs);
	~FuncDeclNode();
	void append(std::string name, ValueTypeS type);
	virtual void accept(Visitor &visitor);

	bool hasArgs;
	std::string *name;
};


class FuncDefNode : public Node {
public:
	FuncDefNode(FuncDeclNode *decl, BlockNode *block);
	~FuncDefNode();
	virtual void accept(Visitor &visitor);

	FuncDeclNode *decl;
	BlockNode *block;
};

class StructDefNode : public Node {
public:
	StructDefNode(std::string *name, NodeList *decls);
	~StructDefNode();
	virtual void accept(Visitor &visitor);

	std::string *name;
	NodeList *decls;
};

class CompUnitNode : public Node {
public:
	CompUnitNode(Node *node);
	~CompUnitNode();
	void append(Node *node);
	virtual void accept(Visitor &visitor);

	list<Node*> nodes;
};

#endif
