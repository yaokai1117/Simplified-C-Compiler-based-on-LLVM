#include <cstdio>
#include <map>
#include <vector>
#include <list>

#include "check_visitor.h"
#include "node.h"
#include "msgfactory.h"

extern MsgFactory msgFactory;
extern bool errorFlag;

extern std::list<Node *> astNodes;

using namespace std;


static void printType(ValueTypeS vType)
{
	if (vType.isConstant)
		printf("const ");
	switch (vType.type) {
	case INT_TYPE:
		printf("int");
		return;
	case FLOAT_TYPE:
		printf("float");
		return;
	case CHAR_TYPE:
		printf("char");
		return;
	case PTR_TYPE:
		printf("pointer( ");
		printType(*(vType.atom));
		printf(" )");
		return;
	case ARRAY_TYPE:
		printf("array( ");
		printType(*(vType.atom));
		printf(" )");
		return;
	case STRUCT_TYPE:
		printf("struct %s", vType.structName->c_str());
		return;
	case FUNC_TYPE:
		printf("( ");
		if (vType.argv != NULL) {
			for (std::list<Node *>::iterator it = vType.argv->nodes.begin();
					it != vType.argv->nodes.end(); it++) {
				printType((*it)->valueTy);
				printf(", ");
			}
		}
		printf(" ) -> ");
		printType(*(vType.atom));
		return;
	case VOID_TYPE:
		printf("void");
		return;
	default:
		return;
	}
}


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
	else if (a->type == STRUCT_TYPE)
		return *(a->structName) == *(b->structName);

	else if (a->type == FUNC_TYPE) {
		if (!typeIsEqual(a->atom, b->atom))
			return false;
		list<Node *> argvA = a->argv->nodes;
		list<Node *> argvB = b->argv->nodes;
		if (argvA.size() != argvB.size())
			return false;
		for (list<Node *>::iterator itA = argvA.begin(), itB = argvB.begin();
				itA != argvA.end(); itA++, itB++) {
			if (!typeIsEqual(&(*itA)->valueTy, &(*itB)->valueTy))
				return false;
		}
		return true;
	}

	else
		return true;
}

static ValueTypeS typeUp(ValueTypeS *ta, ValueTypeS *tb)
{
	ValueTypeS retT;
	retT.type = NO_TYPE;
	return retT;
}

static ExpNode *getSimpleNode(ValueTypeS vType, Loc *loc)
{
	ExpNode *node;
	ConstVal val = vType.constVal;

	switch (vType.type) {
	case INT_TYPE:
		node = new NumNode(val.ival);
		break;
	case FLOAT_TYPE:
		node = new FNumNode(val.fval);
		break;
	case CHAR_TYPE:
		node = new CharNode(val.cval);
		break;
	default:
		return NULL;
	}

	node->valueTy = vType;
	node->setLoc(loc);

	astNodes.push_back(node);

	return node;
}


static bool isAtomType(ValueTypeS vType)
{
	return vType.type == INT_TYPE || vType.type == FLOAT_TYPE || vType.type == CHAR_TYPE;
}


void CheckVisitor::handleArrayType(ValueTypeS *vType)
{
	if (errorFlag)
		return;

	list<Node *> nodes;
	ValueTypeS sizeTy;
	int i;

	switch (vType->type) {
	case ARRAY_TYPE:
		nodes = vType->argv->nodes;

		vType->dim = nodes.size();

		vType->base = new int[vType->dim];
		i = 0;
		for (list<Node *>::iterator it = nodes.begin();
				it != nodes.end(); it++, i++) {

			if ((*it) == NULL) {
				if (it == nodes.begin()) {
					vType->base[0] = 0;
					continue;
				}
				else {
					errorFlag = true;
					return;
				}
			}

			(*it)->accept(*this);

			sizeTy = (*it)->valueTy;

			if (errorFlag)
				return;
			if (sizeTy.type != INT_TYPE) {
				errorFlag = true;
				return;
			}

			if (sizeTy.isComputed && sizeTy.type == INT_TYPE)
				vType->base[i] = sizeTy.constVal.ival;
			else {
				errorFlag = true;
				msgFactory.newError(e_array_size_not_constant, (*it)->loc->first_line, (*it)->loc->first_column);
				return;
			}
		}
		handleArrayType(vType->atom);
		return;
	case PTR_TYPE:
		handleArrayType(vType->atom);
		return;
	case FUNC_TYPE:
		handleArrayType(vType->atom);
		if (errorFlag)
			return;
		nodes = vType->argv->nodes;
		for (list<Node*>::iterator it = nodes.begin();
				it != nodes.end(); it++) {
			handleArrayType(&(*it)->valueTy);
		}
		return;
	default:
		return;
	}
}


ValueTypeS CheckVisitor::lookUpSym(string name)
{
	int sp = stackPtr;
	while (sp > 0) {
		map<string, ValueTypeS> &symTable = *symTableStack[sp-1];
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
	isGlobal = true;
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

	vType.isComputed = true;
	vType.constVal.ival = node->val;
	printType(vType);
	printf("  : Num\n");
}


void CheckVisitor::visitFNumNode(FNumNode *node)
{
	ValueTypeS &vType = node->valueTy;

	vType.type = FLOAT_TYPE;
	vType.isConstant = true;

	vType.isComputed = true;
	vType.constVal.fval = node->fval;
	printType(vType);
	printf("  : Fnum\n");
}


void CheckVisitor::visitCharNode(CharNode *node)
{
	ValueTypeS &vType = node->valueTy;

	vType.type = CHAR_TYPE;
	vType.isConstant = true;

	vType.isComputed = true;
	vType.constVal.cval = node->cval;
	printType(vType);
	printf("  : Char\n");
}


void CheckVisitor::visitBinaryExpNode(BinaryExpNode *node)
{
	if (errorFlag)
		return;

	ValueTypeS &vType = node->valueTy;
	ValueTypeS &lhsTy = node->lhs->valueTy;
	ValueTypeS &rhsTy = node->rhs->valueTy;

	if (!isAtomType(lhsTy) || !isAtomType(rhsTy)) {
		errorFlag = false;
		msgFactory.newError(e_type_unmatch, node->loc->first_line, node->loc->first_column);
		return;
	}

	if (!typeIsEqual(&lhsTy, &rhsTy)) {
		ValueTypeS upTy = typeUp(&lhsTy, &rhsTy);
		if (upTy.type == NO_TYPE) {
			errorFlag = false;
			msgFactory.newError(e_type_unmatch, node->loc->first_line, node->loc->first_column);
			return;
		}
	}

	vType.type = lhsTy.type;

	if (node->op == '%' && vType.type == FLOAT_TYPE) {
		errorFlag = false;
		msgFactory.newError(e_float_mod, node->loc->first_line, node->loc->first_column);
		return;
	}

	if (lhsTy.isComputed)
		node->lhs = getSimpleNode(lhsTy, node->lhs->loc);
	if (rhsTy.isComputed)
		node->rhs = getSimpleNode(rhsTy, node->rhs->loc);

	if (lhsTy.isComputed && rhsTy.isComputed) {
		vType.isComputed = true;
		ConstVal lval = lhsTy.constVal;
		ConstVal rval = rhsTy.constVal;
		switch (vType.type) {
		case INT_TYPE:
			switch (node->op) {
			case '+': vType.constVal.ival = lval.ival + rval.ival; break;
			case '-': vType.constVal.ival = lval.ival - rval.ival; break;
			case '*': vType.constVal.ival = lval.ival * rval.ival; break;
			case '/': vType.constVal.ival = lval.ival / rval.ival; break;
			case '%': vType.constVal.ival = lval.ival % rval.ival; break;
			}
			break;
		case FLOAT_TYPE:
			switch (node->op) {
			case '+': vType.constVal.ival = lval.ival + rval.ival; break;
			case '-': vType.constVal.ival = lval.ival - rval.ival; break;
			case '*': vType.constVal.ival = lval.ival * rval.ival; break;
			case '/': vType.constVal.ival = lval.ival / rval.ival; break;
			}
			break;
		default:
			break;
		}
	}
	printType(vType);
	printf("  : Binary\n");
}


void CheckVisitor::visitUnaryExpNode(UnaryExpNode *node)
{
	if (errorFlag)
	return;

	ValueTypeS &vType = node->valueTy;
	ValueTypeS &operandTy = node->operand->valueTy;


	if (operandTy.isComputed)
		node->operand = getSimpleNode(operandTy, node->operand->loc);


	switch (node->op) {
	case '+':
		if (!isAtomType(operandTy) || operandTy.type == CHAR_TYPE) {
			errorFlag = false;
			msgFactory.newError(e_type_unmatch, node->loc->first_line, node->loc->first_column);
			return;
		}
		vType = operandTy;
		break;
	case '-':
		if (!isAtomType(operandTy) || operandTy.type == CHAR_TYPE) {
			errorFlag = false;
			msgFactory.newError(e_type_unmatch, node->loc->first_line, node->loc->first_column);
			return;
		}
		vType.type = operandTy.type;
		if (operandTy.isComputed) {
			vType.isComputed = true;
			if (operandTy.type == INT_TYPE)
				vType.constVal.ival = -operandTy.constVal.ival;
			else
				vType.constVal.fval = -operandTy.constVal.fval;
		}
		break;
	case '&':
		vType.type = PTR_TYPE;
		vType.atom = &(node->operand->valueTy);
		break;
	case '*':
		if (operandTy.type != PTR_TYPE) {
			errorFlag = false;
			msgFactory.newError(e_type_unmatch, node->loc->first_line, node->loc->first_column);
			return;
		}
		vType = *(operandTy.atom);
		break;
	}
	printType(vType);
	printf("  : Unary\n");
}


void CheckVisitor::visitIdNode(IdNode *node)
{
	if (errorFlag)
		return;

	ValueTypeS vType = lookUpSym(*node->name);
	if (vType.type == NO_TYPE) {
		errorFlag = true;
		msgFactory.newError(e_undeclared_identifier, node->loc->first_line, node->loc->first_column);
		return;
	}

	node->valueTy = vType;

	printType(vType);
	printf("  : Id\n");
}


void CheckVisitor::visitArrayItemNode(ArrayItemNode *node)
{
	if (errorFlag)
		return;

	ValueTypeS &vType = node->valueTy;
	ValueTypeS &arrayTy = node->array->valueTy;
	ValueTypeS &indexTy = node->index->valueTy;

	if (arrayTy.type != ARRAY_TYPE || indexTy.type != INT_TYPE) {
		errorFlag = true;
		printf("array type error\n");
		return;
	}

	// constant propagation
	if (indexTy.isComputed && node->index->type != NUM_AST)
		node->index = getSimpleNode(indexTy, node->index->loc);

	vType = *(arrayTy.atom);

	printType(vType);
	printf("  : ArrayItem\n");
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
	handleArrayType(&vType);

	// if an assignment exists, check the type
	if (node->isAssigned) {
		ValueTypeS &asnTy = node->value->valueTy;
		if (!typeIsEqual(&vType, &asnTy)) {	// type cast
			asnTy.dstType = vType.type;
			msgFactory.newWarning(w_type_cast, node->loc->first_line, node->loc->first_column);
		}
		if (!asnTy.isComputed && isGlobal) {
			errorFlag = true;
			msgFactory.newError(e_global_init_not_constant, node->loc->first_line, node->loc->first_column);
			return;
		}
		if (vType.isConstant && asnTy.isComputed) {						// constant propagation
			node->value = getSimpleNode(asnTy, node->loc);
			vType.isComputed = true;
			vType.constVal = asnTy.constVal;
		}
	}
	else {
		if (vType.isConstant) {
			errorFlag = true;
			msgFactory.newError(e_const_decl_not_init, node->loc->first_line, node->loc->first_column);
			return;
		}
	}

	// global variable
	if (isGlobal) {
		if (globalSymTabble.find(*node->name) != globalSymTabble.end()) {
			errorFlag = true;
			msgFactory.newError(e_redefinition_of_identifier, node->loc->first_line, node->loc->first_column);
			return;
		}
		globalSymTabble[*node->name] = vType;

	}

	// local variable or struct
	else {
		map<string, ValueTypeS> &symTable = *symTableStack[stackPtr-1];
		if (symTable.find(*(node->name)) != symTable.end()) {
			errorFlag = true;
			msgFactory.newError(e_redefinition_of_identifier, node->loc->first_line, node->loc->first_column);
			return;
		}
		symTable[*node->name] = vType;

	}

	printType(vType);
	printf("  : IdDef\n");
}


void CheckVisitor::visitArrayVarDefNode(ArrayVarDefNode *node)
{
	if (errorFlag)
		return;

	ValueTypeS &vType = node->valueTy;

	// handle array
	handleArrayType(&vType);

	// if an assignment exists, check the type
	if (node->isAssigned) {
		list<Node*> nodes = node->values->nodes;
		ValueTypeS *atomTy = vType.atom;
		for (list<Node *>::iterator it = nodes.begin();
				it != nodes.end(); it++) {
			if (!typeIsEqual(atomTy, &(*it)->valueTy)) {
				(*it)->valueTy.dstType = atomTy->type;
				msgFactory.newWarning(w_type_cast, node->loc->first_line, node->loc->first_column);
			}
		}
	}
	else {
		if (vType.isConstant) {
			errorFlag = true;
			msgFactory.newError(e_const_decl_not_init, node->loc->first_line, node->loc->first_column);
			return;
		}
	}

	// global variable
	if (isGlobal) {
		if (globalSymTabble.find(*node->name) != globalSymTabble.end()) {
			errorFlag = true;
			msgFactory.newError(e_redefinition_of_identifier, node->loc->first_line, node->loc->first_column);
			return;
		}
		globalSymTabble[*node->name] = vType;
	}

	// local variable or struct
	else {
		map<string, ValueTypeS> &symTable = *symTableStack[stackPtr-1];
		if (symTable.find(*(node->name)) != symTable.end()) {
			errorFlag = true;
			msgFactory.newError(e_redefinition_of_identifier, node->loc->first_line, node->loc->first_column);
			return;
		}
		symTable[*node->name] = vType;
	}

	printType(vType);
	printf("  : ArraryDef\n");
}


void CheckVisitor::visitEmptyNode(EmptyNode *node)
{
}


void CheckVisitor::visitBlockNode(BlockNode *node)
{
	stackPtr--;
	delete symTableStack[stackPtr];
}


void CheckVisitor::visitVarDeclNode(VarDeclNode *node)
{
}


void CheckVisitor::visitAssignStmtNode(AssignStmtNode *node)
{
}


void CheckVisitor::visitFunCallStmtNode(FunCallStmtNode *node)
{
}


void CheckVisitor::visitBlockStmtNode(BlockStmtNode *node)
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
	isGlobal = true;
}


void CheckVisitor::visitStructDefNode(StructDefNode *node)
{
}


void CheckVisitor::visitCompUnitNode(CompUnitNode *node)
{
}


void CheckVisitor::enterBlockNode(BlockNode *node)
{
	symTableStack[stackPtr] = new map<string, ValueTypeS>;
	stackPtr++;
}


void CheckVisitor::enterIfStmtNode(IfStmtNode *node)
{
}


void CheckVisitor::enterWhileStmtNode(WhileStmtNode *node)
{
}


void CheckVisitor::enterFuncDefNode(FuncDefNode *node)
{
	isGlobal = false;
}


void CheckVisitor::enterStructDefNode(StructDefNode *node)
{
}


