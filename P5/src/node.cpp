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
	int nThis = dumper->newNode(4, name->c_str(), "[", " ", "]");
	int nIndex = index->dumpdot(dumper);
	dumper->drawLine(nThis, 2, nIndex);
	return nThis;
}


// implementation of class ConstDefNode



int main()
{
	return 0;
}

