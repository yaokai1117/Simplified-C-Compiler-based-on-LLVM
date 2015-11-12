#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>
#include <list>
#include "node.h"
#include "dumpdot.h"


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

void NodeList::dumpdot(DumpDOT *dumper, int nRoot, int pos)
{
	for (std::list<Node*>::iterator it = nodes.begin(); it != nodes.end(); it++) {
		int nChild = (*it)->dumpdot(dumper);
		dumper->drawLine(nRoot, pos, nChild);
	}
}

int NodeList::dumpdot(DumpDOT *dumper)
{
	return 0;
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

int NumNode::dumpdot(DumpDOT *dumper)
{
	std::stringstream ss;
	ss << val;
	int nThis = dumper->newNode(1, ss.str().c_str());
	return nThis;
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

int BinaryExpNode::dumpdot(DumpDOT *dumper)
{
	char st[2] = " ";
	st[0] = op;
	int nThis = dumper->newNode(3, " ", st, " ");
	int nlhs = lhs->dumpdot(dumper);
	int nrhs = rhs->dumpdot(dumper);
	dumper->drawLine(nThis, 0, nlhs);
	dumper->drawLine(nThis, 2, nrhs);
	return nThis;
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

int UnaryExpNode::dumpdot(DumpDOT *dumper)
{
	char st[2] = " ";
	st[0] = op;
	int nThis = dumper->newNode(2, st, " ");
	int nOperand = operand->dumpdot(dumper);
	dumper->drawLine(nThis, 1, nOperand);
	return nThis;
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

int IdNode::dumpdot(DumpDOT *dumper)
{
	int nThis = dumper->newNode(1, name->c_str());
	return nThis;
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

int ArrayItemNode::dumpdot(DumpDOT *dumper)
{
	int nThis = dumper->newNode(4, name->c_str(), "\\[", " ", "\\]");
	int nIndex = index->dumpdot(dumper);
	dumper->drawLine(nThis, 2, nIndex);
	return nThis;
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

int IdConstDefNode::dumpdot(DumpDOT *dumper)
{
	int nThis = dumper->newNode(3, name->c_str(), "=", " ");
	int nValue = value->dumpdot(dumper);
	dumper->drawLine(nThis, 2, nValue);
	return nThis;
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

int ArrayConstDefNode::dumpdot(DumpDOT *dumper)
{
	int nThis = dumper->newNode(8, name->c_str(), "\\[", " ", "\\]", "=", "\\{", " ", "\\}");
	if (hasSize) {
		int nSize = size->dumpdot(dumper);
		dumper->drawLine(nThis, 2, nSize);
	}
	values->dumpdot(dumper, nThis, 6);
	return nThis;
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

int IdVarDefNode::dumpdot(DumpDOT *dumper)
{
	int nThis = 0;
	if (isAssigned) {
		nThis = dumper->newNode(3, name->c_str(), "=", " ");
		int nValue = value->dumpdot(dumper);
		dumper->drawLine(nThis, 2, nValue);
	}
	else {
		nThis = dumper->newNode(1, name->c_str());
	}
	return nThis;
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

int ArrayVarDefNode::dumpdot(DumpDOT *dumper)
{
	int nThis = 0;
	if (isAssigned) {
		nThis = dumper->newNode(8, name->c_str(), "\\[", " ", "\\]", "=", "\\{", " ", "\\}");
		if (hasSize) {
			int nSize = size->dumpdot(dumper);
			dumper->drawLine(nThis, 2, nSize);
		}
		values->dumpdot(dumper, nThis, 6);
	}
	else {
		nThis = dumper->newNode(4, name->c_str(), "\\[", " ", "\\]");
		int nSize = size->dumpdot(dumper);
		dumper->drawLine(nThis, 2, nSize);
	}
	return nThis;
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

int BlockNode::dumpdot(DumpDOT *dumper)
{
	int nThis = dumper->newNode(3, "\\{", " ", "\\}");
	blockItems->dumpdot(dumper, nThis, 1);
	return nThis;
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

int AssignStmtNode::dumpdot(DumpDOT *dumper)
{
	int nThis = dumper->newNode(3, " ", "=", " ");
	int nLVal = lval->dumpdot(dumper);
	int nExp = exp->dumpdot(dumper);
	dumper->drawLine(nThis, 0, nLVal);
	dumper->drawLine(nThis, 2, nExp);
	return nThis;
}


// implementation of class FunCallStmtNode
FunCallStmtNode::FunCallStmtNode(std::string *name)
	: name(name)
{
	type = FUNCALL_STMT_AST;
}

FunCallStmtNode::~FunCallStmtNode()
{
	delete name;
}

int FunCallStmtNode::dumpdot(DumpDOT *dumper)
{
	int nThis = dumper->newNode(2, name->c_str(), "\\( \\)");
	return nThis;
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

int BlockStmtNode::dumpdot(DumpDOT *dumper)
{
	return block->dumpdot(dumper);
}


// implementation of class CondNode
CondNode::CondNode(OpType op, ExpNode *lhs, ExpNode *rhs)
	: op(op), lhs(lhs), rhs(rhs)
{
	type = COND_AST;
}

CondNode::~CondNode()
{
}

int CondNode::dumpdot(DumpDOT *dumper)
{
	char op_str[4] = "   ";	
	switch (op) {
		case LT_OP:
			op_str[0] = '\\';
			op_str[1] = '<';
			break;
		case LTE_OP:
			op_str[0] = '\\';
			op_str[1] = '<';
			op_str[2] = '=';
			break;
		case GT_OP:
			op_str[0] = '\\';
			op_str[1] = '>';
			break;
		case GTE_OP:
			op_str[0] = '\\';
			op_str[1] = '>';
			op_str[2] = '=';
			break;
		case EQ_OP:
			op_str[1] = '=';
			op_str[2] = '=';
			break;
		case NEQ_OP:
			op_str[1] = '!';
			op_str[2] = '=';
			break;
		default:
			break;
	}
	int nThis = dumper->newNode(3, " ", op_str, " ");
	int nLhs = lhs->dumpdot(dumper);
	int nRhs = rhs->dumpdot(dumper);
	dumper->drawLine(nThis, 0, nLhs);
	dumper->drawLine(nThis, 2, nRhs);
	return nThis;
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

int IfStmtNode::dumpdot(DumpDOT *dumper)
{
	int nThis = 0;
	if (hasElse) {
		nThis = dumper->newNode(6, "if", " ", "then", " ", "else", " ");
		int nCond = cond->dumpdot(dumper);
		int nThen = then_stmt->dumpdot(dumper);
		int nElse = else_stmt->dumpdot(dumper);
		dumper->drawLine(nThis, 1, nCond);
		dumper->drawLine(nThis, 3, nThen);
		dumper->drawLine(nThis, 5, nElse);
	}
	else {
		nThis = dumper->newNode(4, "if", " ", "then", " ");
		int nCond = cond->dumpdot(dumper);
		int nThen = then_stmt->dumpdot(dumper);
		dumper->drawLine(nThis, 1, nCond);
		dumper->drawLine(nThis, 3, nThen);
	}
	return nThis;
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

int WhileStmtNode::dumpdot(DumpDOT *dumper)
{
	int nThis = dumper->newNode(3, "while", " ", " ");
	int nCond = cond->dumpdot(dumper);
	int nDo = do_stmt->dumpdot(dumper);
	dumper->drawLine(nThis, 1, nCond);
	dumper->drawLine(nThis, 2, nDo);
	return nThis;
}


// implementation of class FuncDefNode
FuncDefNode::FuncDefNode(std::string *name, BlockNode *block)
	: name(name), block(block)
{
	type = FUNC_DEF_AST;
}

FuncDefNode::~FuncDefNode()
{
	delete name;
}

int FuncDefNode::dumpdot(DumpDOT *dumper)
{
	int nThis = dumper->newNode(4, "void", name->c_str(), "\\( \\)", " ");
	int nBlock = block->dumpdot(dumper);
	dumper->drawLine(nThis, 3, nBlock);
	return nThis;
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


int ConstDeclNode::dumpdot(DumpDOT *dumper)
{
	int nThis = dumper->newNode(3, "const", "int", " ");
	defList->dumpdot(dumper, nThis, 2);
	return nThis;
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

int VarDeclNode::dumpdot(DumpDOT *dumper)
{
	int nThis = dumper->newNode(2, "int", " ");
	defList->dumpdot(dumper, nThis, 1);
	return nThis;
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

int CompUnitNode::dumpdot(DumpDOT *dumper)
{
	int nThis = dumper->newNode(1, "CompUnit");
	for (std::list<Node*>::iterator it = nodes.begin(); it != nodes.end(); it++) {
		int nUnit = (*it)->dumpdot(dumper);
		dumper->drawLine(nThis, 0, nUnit);
	}
	return nThis;
}



/*
int main()
{
	return 0;
}
*/

