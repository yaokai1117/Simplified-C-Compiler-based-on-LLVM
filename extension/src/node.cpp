#include <cstdio>
#include <cstdlib>
#include <string>
#include <list>
#include "node.h"
#include "visitor.h"


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

void NodeList::accept(Visitor &v)
{
	for (std::list<Node *>::iterator it = nodes.begin();
			it != nodes.end(); it++) {
		(*it)->accept(v);
	}
	v.visitNodeList(this);
}



// implementation of class NumNode
NumNode::NumNode(int val)
	: val(val)
{
	type = NUM_AST;
}

NumNode::~NumNode()
{
}

void NumNode::accept(Visitor &v)
{
	v.visitNumNode(this);
}


// implementation of class FNumNode
FNumNode::FNumNode(double fval)
	: fval(fval)
{
	type = FNUM_AST;
}

FNumNode::~FNumNode()
{
}

void FNumNode::accept(Visitor &v)
{
	v.visitFNumNode(this);
}


// implementation of class CharNode
CharNode::CharNode(char cval)
	: cval(cval)
{
	type = CHAR_AST;
}

CharNode::~CharNode()
{
}

void CharNode::accept(Visitor &v)
{
	v.visitCharNode(this);
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

void BinaryExpNode::accept(Visitor &v)
{
	lhs->accept(v);
	rhs->accept(v);
	v.visitBinaryExpNode(this);
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

void UnaryExpNode::accept(Visitor &v)
{
	operand->accept(v);
	v.visitUnaryExpNode(this);
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

void IdNode::accept(Visitor &v)
{
	v.visitIdNode(this);
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

void ArrayItemNode::accept(Visitor &v)
{
	index->accept(v);
	v.visitArrayItemNdoe(this);
}


// implementation of class FunCallNode
FunCallNode::FunCallNode(std::string *name, NodeList *argv)
	: name(name), argv(argv)
{
	type = FUN_CALL_AST;
	hasArgs = (argv != NULL);
}

FunCallNode::~FunCallNode()
{
	delete name;
}

void FunCallNode::accept(Visitor &v)
{
	if (hasArgs) {
		argv->accept(v);
	}
	v.visitFunCallNode(this);
}


// implementation of class IdVarDefNode
IdVarDefNode::IdVarDefNode(std::string *name, ExpNode *value=NULL)
	: value(value)
{
	type = ID_VAR_DEF_AST;
	isAssigned = (value != NULL);

	this->name = name;
}

IdVarDefNode::~IdVarDefNode()
{
	delete name;
}

void IdVarDefNode::accept(Visitor &v)
{
	if (isAssigned) {
		value->accept(v);
	}
	v.visitIdVarDefNode(this);
}


// implementation of class ArrayVarDefNode
ArrayVarDefNode::ArrayVarDefNode(std::string *name, ExpNode *size, NodeList *values=NULL)
	: size(size), values(values)
{
	type = ARRAY_VAR_DEF_AST;
	isAssigned = (values != NULL);
	hasSize = (size != NULL);

	this->name = name;
}

ArrayVarDefNode::~ArrayVarDefNode()
{
	delete name;
}

void ArrayVarDefNode::accept(Visitor &v)
{
	if (hasSize) {
		size->accept(v);
	}

	if (isAssigned) {
		values->accept(v);
	}

	v.visitArrayVarDefNode(this);
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

void BlockNode::accept(Visitor &v)
{
	v.enterBlockNode(this);
	blockItems->accept(v);
	v.visitBlockNode(this);
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

void AssignStmtNode::accept(Visitor &v)
{
	// In some cases (such as code generation), the visiting order should be changed
	if (v.orderChanged) {
		v.visitAssignStmtNode(this);
		return;
	}

	lval->accept(v);
	exp->accept(v);
	v.visitAssignStmtNode(this);
}


// implementation of class FunCallStmtNode
FunCallStmtNode::FunCallStmtNode(FunCallNode *funCall)
	: funCall(funCall)
{
	type = FUNCALL_STMT_AST;
}

FunCallStmtNode::~FunCallStmtNode()
{
}

void FunCallStmtNode::accept(Visitor &v)
{
	funCall->accept(v);
	v.visitFunCallStmtNode(this);
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

void BlockStmtNode::accept(Visitor &v)
{
	block->accept(v);
	v.visitBlockStmtNode(this);
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

void CondNode::accept(Visitor &v)
{
	// In some cases (such as code generation), the visiting order should be changed
	if (v.orderChanged) {
		v.visitCondNode(this);
		return;
	}

	lhs->accept(v);
	rhs->accept(v);
	v.visitCondNode(this);
}


// implementation of class EmptyNode
EmptyNode::EmptyNode()
{
}

EmptyNode::~EmptyNode()
{
}

void EmptyNode::accept(Visitor &v)
{
	v.visitEmptyNode(this);
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

void IfStmtNode::accept(Visitor &v)
{
	// In some cases (such as code generation), the visiting order should be changed
	if (v.orderChanged) {
		v.visitIfStmtNode(this);
		return;
	}

	v.enterIfStmtNode(this);

	cond->accept(v);
	then_stmt->accept(v);

	if (hasElse) {
		else_stmt->accept(v);
	}

	v.visitIfStmtNode(this);
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

void WhileStmtNode::accept(Visitor &v)
{
	// In some cases (such as code generation), the visiting order should be changed
	if (v.orderChanged) {
		v.visitWhileStmtNdoe(this);
		return;
	}

	v.enterWhileStmtNode(this);

	cond->accept(v);
	do_stmt->accept(v);

	v.visitWhileStmtNdoe(this);
}


ReturnStmtNode::ReturnStmtNode(ExpNode *exp)
	: exp(exp)
{
	type = RETURN_STMT_AST;
}

ReturnStmtNode::~ReturnStmtNode()
{
}

void ReturnStmtNode::accept(Visitor &v)
{
	exp->accept(v);
	v.visitReturnStmtNdoe(this);
}


// implementation of class BreakStmtNode
BreakStmtNode::BreakStmtNode()
{
}
BreakStmtNode::~BreakStmtNode()
{
}

void BreakStmtNode::accept(Visitor &v)
{
	v.visitBreakStmtNode(this);
}


// implementation of class ContinueStmtNode
ContinueStmtNode::ContinueStmtNode()
{
}

ContinueStmtNode::~ContinueStmtNode()
{
}

void ContinueStmtNode::accept(Visitor &v)
{
	v.visitContinueStmtNode(this);
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

void FuncDeclNode::accept(Visitor &v)
{
	// In some cases (such as code generation), the visiting order should be changed
	if (v.orderChanged) {
		v.visitFuncDeclNode(this);
		return;
	}

	if (hasArgs) {
		argv->accept(v);
	}
	v.visitFuncDeclNode(this);
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

void FuncDefNode::accept(Visitor &v)
{
	// In some cases (such as code generation), the visiting order should be changed
	if (v.orderChanged) {
		v.visitFuncDefNode(this);
		return;
	}

	v.enterFuncDefNode(this);

	decl->accept(v);
	block->accept(v);

	v.visitFuncDefNode(this);
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

void ConstDeclNode::accept(Visitor &v)
{
	defList->accept(v);

	v.visitConstDeclNode(this);
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

void VarDeclNode::accept(Visitor &v)
{
	defList->accept(v);

	v.visitVarDeclNode(this);
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

void CompUnitNode::accept(Visitor &v)
{
	for (std::list<Node *>::iterator it = nodes.begin();
			it != nodes.end(); it++) {
		(*it)->accept(v);
	}

	v.visitCompUnitNode(this);
}



/*
int main()
{
	return 0;
}
*/

