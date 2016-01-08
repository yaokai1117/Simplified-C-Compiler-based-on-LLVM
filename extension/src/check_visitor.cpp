#include <cstdio>
#include <map>
#include <vector>
#include <list>

#include "check_visitor.h"
#include "node.h"
#include "msgfactory.h"

extern MsgFactory msgFactory;
extern bool errorFlag;

using namespace std;


static bool typeIsEqual(ValueTypeS *a, ValueTypeS *b)
{
	if (a->type != b->type)
		return false;

	if (a->type == ARRAY_TYPE) {
		if (a->dim != b->dim)
			return false;
		for (int i = 0; i < a->dim; i++)
			if (a->base[i] != b->base[i])
				return false;
		return typeIsEqual(a->atom, b->atom);
	}
	else if (a->type == PTR_TYPE)
		return typeIsEqual(a->atom, b->atom);
	else
		return true;
}


void CheckVisitor::handleArrayType(ValueTypeS *vType)
{
	if (vType->type != ARRAY_TYPE)
		return;

	list<Node *> nodes = vType->base->nodes;

	vType->dim = nodes.size();

	vType->base = new int[vType->dim];
	int i = 0;
	for (list<Node *>::iterator it = nodes.begin();
			it != nodes.end(); it++, i++) {

		if ((*it) == NULL) {
			if (it == nodes.begin())
				vType->base[0] = 0;
			else {
				errorFlag = true;
				return;
			}
		}

		(*it)->accept(*this);
		if (errorFlag)
			return;
		if ((*it)->valueTy.isConstant && (*it)->valueTy.type == INT_TYPE)
			vType->base[i] = (*it)->constVal.ival;
		else {
			errorFlag = true;
			msgFactory.newError(e_array_size_not_constant, (*it)->loc->first_line, (*it)->loc->first_column);
			return;
		}
	}
}


ValueTypeS CheckVisitor::lookUpSym(string name)
{
	int sp = stackPtr;
	while (sp > 0) {
		map<string, ValueTypeS> &symTable = symTableStack[sp-1];
		if (symTable.find(name) != symTable.end())
			return symTable[name];
		sp--;
	}

	if (globalSymTabble.find(name) != globalSymTabble.end())
		return globalSymTabble[name];

	ValueTypeS retType;
	retType.type = NO_TYPE;
	return retType;
}


CheckVisitor::CheckVisitor()
{
	stackPtr = 0;
	orderChanged = false;
}


CheckVisitor::~CheckVisitor()
{
}


void CheckVisitor::visitNodeList(NodeList *node)
{
}


void CheckVisitor::visitNumNode(NumNode *node)
{
	ValueTypeS &vType = node->valueTy;

	vType.type = INT_TYPE;
	vType.isConstant = true;
}


void CheckVisitor::visitFNumNode(FNumNode *node)
{
	ValueTypeS &vType = node->valueTy;

	vType.type = FLOAT_TYPE;
	vType.isConstant = true;
}


void CheckVisitor::visitCharNode(CharNode *node)
{
	ValueTypeS &vType = node->valueTy;

	vType.type = CHAR_TYPE;
	vType.isConstant = true;
}


void CheckVisitor::visitBinaryExpNode(BinaryExpNode *node)
{
	// to be completed
}


void CheckVisitor::visitUnaryExpNode(UnaryExpNode *node)
{
	// to be completed
}


void CheckVisitor::visitIdNode(IdNode *node)
{
}


void CheckVisitor::visitArrayItemNode(ArrayItemNode *node)
{
}


void CheckVisitor::visitStructItemNode(StructItemNode *node)
{
}



void CheckVisitor::visitFunCallNode(FunCallNode *node)
{
}


void CheckVisitor::visitIdVarDefNode(IdVarDefNode *node)
{
	if (errorFlag)
		return;

	ValueTypeS &vType = node->valueTy;

	// handle array
	ValueTypeS *pt = &vType;
	while (pt->type != ATOM_TYPE) {
		if (pt->type == ARRAY_TYPE) {
			handleArrayType(pt);
			if (pt->base[0] == 0) {
				errorFlag = true;
				msgFactory.newError(e_array_size_not_constant, node->loc->first_line, node->loc->first_column);
				return;
			}
		}
	}



	map<string, ValueTypeS> &symTable = symTableStack[stackPtr-1];
	if (symTable.find(*(node->name)) != symTable.end()) {

	}


	if (node->isAssigned) {
		if (typeIsEqual(vType, node->value->valueTy))

	}
}


void CheckVisitor::visitArrayVarDefNode(ArrayVarDefNode *node)
{
}


void CheckVisitor::visitEmptyNode(EmptyNode *node)
{
}


void CheckVisitor::visitBlockNode(BlockNode *node)
{
}


void CheckVisitor::visitVarDeclNode(VarDeclNode *node)
{
}


void CheckVisitor::visitAssignStmtNode(AssignStmtNode *node)
{
}


void DumpDotVisitor::visitFunCallStmtNode(FunCallStmtNode *node)
{
}


void DumpDotVisitor::visitBlockStmtNode(BlockStmtNode *node)
{
}


void CheckVisitor::visitCondNode(CondNode *node)
{
}


void CheckVisitor::visitIfStmtNode(IfStmtNode *node)
{
}


void CheckVisitor::visitWhileStmtNdoe(WhileStmtNode *node)
{
}


void CheckVisitor::visitReturnStmtNdoe(ReturnStmtNode *node)
{
}


void CheckVisitor::visitBreakStmtNode(BreakStmtNode *node)
{
}


void CheckVisitor::visitContinueStmtNode(ContinueStmtNode *node)
{
}


void CheckVisitor::visitFuncDeclNode(FuncDeclNode *node)
{
}


void CheckVisitor::visitFuncDefNode(FuncDefNode *node)
{
}


void CheckVisitor::visitStructDefNode(StructDefNode *node)
{
}


void CheckVisitor::visitCompUnitNode(CompUnitNode *node)
{
}


void CheckVisitor::enterBlockNode(BlockNode *node)
{
}


void CheckVisitor::enterIfStmtNode(IfStmtNode *node)
{
}


void CheckVisitor::enterWhileStmtNode(WhileStmtNode *node)
{
}


void CheckVisitor::enterFuncDefNode(FuncDefNode *node)
{
}


void CheckVisitor::enterStructDefNode(StructDefNode *node)
{
}


