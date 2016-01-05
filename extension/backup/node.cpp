#include <cstdio>
#include <cstdlib>
#include <string>
#include <list>
#include "node.h"


// implementation of class Node
Node::Node()
{
	loc = new Loc;
}

Node::~Node()
{
	delete loc;
}


void Node::setLoc(Loc *loc)
{
	this->loc->first_line 	= loc->first_line;
	this->loc->first_column = loc->first_column;
	this->loc->last_line 	= loc->last_line;
	this->loc->last_column 	= loc->last_column;
}


// implementation of class NodeList
NodeList::NodeList(Node *node)
{
	nodes.push_back(node);
}

NodeList::~NodeList()
{
}

void NodeList::append(Node *node)
{
	nodes.push_back(node);
}



// implementation of class NumNode
NumNode::NumNode(int val)
	: val(val)
{
	type = NUM_AST;
	valTy = INT_TYPE;
}

NumNode::~NumNode()
{
}



// implementation of class FNumNode
FNumNode::FNumNode(double fval)
	: fval(fval)
{
	type = FNUM_AST;
	valTy = FLOAT_TYPE;
}

FNumNode::~FNumNode()
{
}


// implementation of class CharNode
CharNode::CharNode(char cval)
	: cval(cval)
{
	type = CHAR_AST;
	valTy = CHAR_TYPE;
}

CharNode::~CharNode()
{
}


// implementation of class BinaryExpNode
BinaryExpNode::BinaryExpNode(char op, ExpNode *lhs, ExpNode *rhs)
	: op(op), lhs(lhs), rhs(rhs)
{
	type = BINARY_EXP_AST;
}

BinaryExpNode::~BinaryExpNode()
{
}


// implementation of class BinaryExpNode
UnaryExpNode::UnaryExpNode(char op, ExpNode *operand)
	: op(op), operand(operand)
{
	type = UNARY_EXP_AST;
}

UnaryExpNode::~UnaryExpNode()
{
}



// implementation of class IdNode
IdNode::IdNode(std::string *name)
	: name(name)
{
	type = ID_AST;
}

IdNode::~IdNode()
{
	delete name;
}



// implementation of class ArrayItemNode
ArrayItemNode::ArrayItemNode(std::string *name, ExpNode *index)
	: name(name), index(index)
{
	type = ARRAY_ITEM_AST;
}

ArrayItemNode::~ArrayItemNode()
{
	delete name;
}



// implementation of class IdConstDefNode
IdConstDefNode::IdConstDefNode(std::string *name, ExpNode *value)
	: name(name), value(value)
{
	type = ID_CONST_DEF_AST;
}

IdConstDefNode::~IdConstDefNode()
{
	delete name;
}



// implementation of class ArrayConstDefNode
ArrayConstDefNode::ArrayConstDefNode(std::string *name, ExpNode *size, NodeList *values)
	: name(name), size(size), values(values)
{
	type = ARRAY_CONST_DEF_AST;
	if (size == NULL)
		hasSize = false;
	else
		hasSize = true;
}

ArrayConstDefNode::~ArrayConstDefNode()
{
	delete name;
}



// implementation of class IdVarDefNode
IdVarDefNode::IdVarDefNode(std::string *name, ExpNode *value=NULL)
	: name(name), value(value)
{
	type = ID_VAR_DEF_AST;
	isAssigned = (value != NULL);
}

IdVarDefNode::~IdVarDefNode()
{
	delete name;
}



// implementation of class ArrayVarDefNode
ArrayVarDefNode::ArrayVarDefNode(std::string *name, ExpNode *size, NodeList *values=NULL)
	: name(name), size(size), values(values)
{
	type = ARRAY_VAR_DEF_AST;
	isAssigned = (values != NULL);
	hasSize = (size != NULL);
}

ArrayVarDefNode::~ArrayVarDefNode()
{
	delete name;
}



// implementation of class BlockNode
BlockNode::BlockNode(NodeList *blockItems)
	: blockItems(blockItems)
{
	type = BLOCK_AST;	
}

BlockNode::~BlockNode()
{
}



// implementation of class AssignStmtNode
AssignStmtNode::AssignStmtNode(LValNode *lval, ExpNode *exp)
	: lval(lval), exp(exp)
{
	type = ASSIGN_STMT_AST;
}

AssignStmtNode::~AssignStmtNode()
{
}



// implementation of class FunCallStmtNode
FunCallStmtNode::FunCallStmtNode(std::string *name, NodeList *argv)
	: name(name), argv(argv)
{
	type = FUNCALL_STMT_AST;
	hasArgs = (argv != NULL);
}

FunCallStmtNode::~FunCallStmtNode()
{
	delete name;
}



// implementation of class BlockStmtNode
BlockStmtNode::BlockStmtNode(BlockNode *block)
	: block(block)
{
	type = BLOCK_STMT_AST;
}

BlockStmtNode::~BlockStmtNode()
{
}



// implementation of class CondNode
CondNode::CondNode(OpType op, Node *lhs, Node *rhs)
	: op(op), lhs(lhs), rhs(rhs)
{
	type = COND_AST;
}

CondNode::~CondNode()
{
}


// implementation of class EmptyNode
EmptyNode::EmptyNode()
{
}

EmptyNode::~EmptyNode()
{
}


// implementation of class IfStmtNode
IfStmtNode::IfStmtNode(CondNode *cond, StmtNode *then_stmt, StmtNode *else_stmt)
	: cond(cond), then_stmt(then_stmt), else_stmt(else_stmt)
{
	type = IF_STMT_AST;
	hasElse = (else_stmt != NULL);
}

IfStmtNode::~IfStmtNode()
{
}



// implementation of class WhileStmtNode
WhileStmtNode::WhileStmtNode(CondNode *cond, StmtNode *do_stmt)
	: cond(cond), do_stmt(do_stmt)
{
	type = WHILE_STMT_AST;
}

WhileStmtNode::~WhileStmtNode()
{
}

// implementation of class BreakStmtNode
BreakStmtNode::BreakStmtNode()
{
}
BreakStmtNode::~BreakStmtNode()
{
}

// implementation of class ContinueStmtNode
ContinueStmtNode::ContinueStmtNode()
{
}

ContinueStmtNode::~ContinueStmtNode()
{
}


// implemantatian of class FuncDeclNode
FuncDeclNode::FuncDeclNode(string *name, NodeList *argv)
	: name(name), argv(argv)
{
	type = FUNC_DECL_AST;
	hasArgs = (argv != NULL);
}

FuncDeclNode::~FuncDeclNode()
{
	delete name;
}



// implementation of class FuncDefNode
FuncDefNode::FuncDefNode(FuncDeclNode *decl, BlockNode *block)
	: decl(decl), block(block)
{
	type = FUNC_DEF_AST;
}

FuncDefNode::~FuncDefNode()
{
}



// implementation of class ConstDeclNode
ConstDeclNode::ConstDeclNode(NodeList *defList)
	: defList(defList)
{
	type = CONST_DECL_AST;
}

ConstDeclNode::~ConstDeclNode()
{
}



// implementation of class VarDeclNode
VarDeclNode::VarDeclNode(NodeList *defList)
	: defList(defList)
{
	type = VAR_DECL_AST;
}

VarDeclNode::~VarDeclNode()
{
}



// implementation of class CompUnitNode
CompUnitNode::CompUnitNode(Node *node)
{
	nodes.push_back(node);
}

CompUnitNode::~CompUnitNode()
{
}

void CompUnitNode::append(Node *node)
{
	nodes.push_back(node);
}



/*
int main()
{
	return 0;
}
*/

