#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/PassManager.h"

#include "llvm/Analysis/Passes.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"

#include <cctype>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "node.h"
#include "codegen_visitor.h"

using namespace llvm;


extern Module *TheModule;
extern ExecutionEngine *TheExecutionEngine;
extern FunctionPassManager *TheFPM;
static IRBuilder<> Builder(llvm::getGlobalContext());


static ValueTypeS getArrayItemType(ValueTypeS arrayTy)
{
	ValueTypeS vType;

	if (arrayTy.dim == 1)
		vType = *(arrayTy.atom);
	else {
		vType = arrayTy;
		vType.dim--;
		for (int i = 0; i < vType.dim ; i++)
			vType.base[i] = vType.base[i+1];
	}

	return vType;
}

static GlobalVariable::LinkageTypes getLinkageTyp(ValueTypeS vType)
{
	if (vType.isStatic)
		return GlobalVariable::InternalLinkage;
	else
		return GlobalVariable::ExternalLinkage;
}


static Type *getLLVMVarType(ValueTypeS vType)
{
	switch (vType.type) {
	case NO_TYPE:
		return nullptr;
	case INT_TYPE:
		return Type::getInt32Ty(getGlobalContext());
	case FLOAT_TYPE:
		return Type::getFloatTy(getGlobalContext());
	case CHAR_TYPE:
		return Type::getInt8Ty(getGlobalContext());
	case VOID_TYPE:
		return Type::getVoidTy(getGlobalContext());
	case STRUCT_TYPE:
		return TheModule->getTypeByName(*vType.structName);
	case PTR_TYPE:
		return PointerType::get(getLLVMVarType(*vType.atom), 0);
	case ARRAY_TYPE:
		return ArrayType::get(getLLVMVarType(getArrayItemType(vType)), vType.base[0]);
	case FUNC_TYPE:
		{
		std::vector<Type *> types;
		if (vType.argv != NULL) {
			std::list<Node *> nodes = vType.argv->nodes;
			for (std::list<Node *>::iterator it = nodes.begin();
					it != nodes.end(); ++it) {
				types.push_back(getLLVMVarType((*it)->valueTy));
			}
		}
		return FunctionType::get(getLLVMVarType(*vType.atom), types, false);
		}
	default:
		return nullptr;
	}
}


static Value *typeCast(ValueTypeS vType, Value *v)
{
	switch (vType.type) {
	case INT_TYPE:
		if (vType.dstType == FLOAT_TYPE)		// int to float
			v = Builder.CreateCast(Instruction::SIToFP, v, Type::getFloatTy(getGlobalContext()));
		else 									// int to char
			v = Builder.CreateCast(Instruction::Trunc, v, Type::getInt8Ty(getGlobalContext()));
		break;
	case FLOAT_TYPE:
		if (vType.dstType == INT_TYPE)		// float to int
			v = Builder.CreateCast(Instruction::FPToSI, v, Type::getInt32Ty(getGlobalContext()));
		else								// float to char
			v = Builder.CreateCast(Instruction::FPToSI, v, Type::getInt8Ty(getGlobalContext()));
		break;
	case CHAR_TYPE:
		if (vType.dstType == INT_TYPE)		// char to int
			v = Builder.CreateCast(Instruction::SExt, v, Type::getInt32Ty(getGlobalContext()));
		else								// char to float
			v = Builder.CreateCast(Instruction::SIToFP, v, Type::getFloatTy(getGlobalContext()));
		break;
	default:
		break;
	}

	return v;
}


Value *CodegenVisitor::lookUp(std::string nameStr)
{
	Value *retV = nullptr;
	int sp = StackPtr;
	while (sp > 0) {
		std::map<std::string, AllocaInst *> &ConstLocalVariables = *ConstLocalTableStack[sp-1];
		std::map<std::string, AllocaInst *> &LocalVariables = *LocalTableStack[sp-1];
		if (ConstLocalVariables.find(nameStr) != ConstLocalVariables.end()) {
			retV = ConstLocalVariables[nameStr];
			break;
		}
		else if (LocalVariables.find(nameStr) != LocalVariables.end()) {
			retV = LocalVariables[nameStr];
			break;
		}
		else {
			retV = nullptr;
			sp--;
		}
	}
	if (retV == nullptr) {
		if (GloblalVariables.find(nameStr) != GloblalVariables.end()) {
			retV = GloblalVariables[nameStr];
		}
		else
			return 0;
	}
	return retV;
}

std::vector<Value *> CodegenVisitor::getValuesFromStack(int size)
{
	std::vector<Value *> v(size);
	for (int i = 0; i < size; i++) {
		v[size-1-i] = pending.back();
		pending.pop_back();
	}
	return v;
}

// initialization
CodegenVisitor::CodegenVisitor(std::string output_filename)
{
	StackPtr = 0;
	orderChanged = true;
}


CodegenVisitor::~CodegenVisitor()
{
}

void CodegenVisitor::dump()
{
	TheModule->dump();
}


void CodegenVisitor::visitNodeList(NodeList *node)
{
}


void CodegenVisitor::visitNumNode(NumNode *node)
{
	Value *v = ConstantInt::get(getGlobalContext(), APInt(32, node->val, true));
	ValueTypeS vType = node->valueTy;
	if (vType.dstType != NO_TYPE && vType.dstType != vType.type) {
		if (vType.dstType == FLOAT_TYPE)		// int to float
			v = Builder.CreateCast(Instruction::SIToFP, v, Type::getFloatTy(getGlobalContext()));
		else									// int to char
			v = Builder.CreateCast(Instruction::Trunc, v, Type::getInt8Ty(getGlobalContext()));
	}
	pending.insert(pending.end(), v);
}


void CodegenVisitor::visitFNumNode(FNumNode *node)
{
	Value *v = ConstantFP::get(getGlobalContext(), APFloat((float)node->fval));
	ValueTypeS vType = node->valueTy;
	if (vType.dstType != NO_TYPE && vType.dstType != vType.type) {
		if (vType.dstType == INT_TYPE)		// float to int
			v = Builder.CreateCast(Instruction::FPToSI, v, Type::getInt32Ty(getGlobalContext()));
		else								// float to char
			v = Builder.CreateCast(Instruction::FPToSI, v, Type::getInt8Ty(getGlobalContext()));
	}
	pending.insert(pending.end(), v);
}


void CodegenVisitor::visitCharNode(CharNode *node)
{
	Value *v = ConstantInt::get(getGlobalContext(), APInt(8, (int)(node->cval), true));
	ValueTypeS vType = node->valueTy;
	if (vType.dstType != NO_TYPE && vType.dstType != vType.type) {
		if (vType.dstType == INT_TYPE)		// char to int
			v = Builder.CreateCast(Instruction::SExt, v, Type::getInt32Ty(getGlobalContext()));
		else								// char to float
			v = Builder.CreateCast(Instruction::SIToFP, v, Type::getFloatTy(getGlobalContext()));
	}
	pending.insert(pending.end(), v);
}


void CodegenVisitor::visitBinaryExpNode(BinaryExpNode *node)
{
	Value *rValue = pending.back();
	pending.pop_back();
	Value *lValue = pending.back();
	pending.pop_back();

	if (lValue == 0 || rValue == 0) {
		pending.insert(pending.end(), 0);
		return;
	}

	Value *v;
	ValueTypeS vType = node->valueTy;
	if (vType.type == INT_TYPE) {
		switch (node->op) {
		case '+':
			v = Builder.CreateAdd(lValue, rValue, "addtmp");
			break;
		case '-':
			v = Builder.CreateSub(lValue, rValue, "subtmp");
			break;
		case '*':
			v = Builder.CreateMul(lValue, rValue, "multmp");
			break;
		case '/':
			v = Builder.CreateSDiv(lValue, rValue, "divtmp");
			break;
		case '%':
			v = Builder.CreateSRem(lValue, rValue, "modtmp");
			break;
		default:
			v = 0;
			break;
		}
	}
	else {
		switch (node->op) {
		case '+':
			v = Builder.CreateFAdd(lValue, rValue, "addtmp");
			break;
		case '-':
			v = Builder.CreateFSub(lValue, rValue, "subtmp");
			break;
		case '*':
			v = Builder.CreateFMul(lValue, rValue, "multmp");
			break;
		case '/':
			v = Builder.CreateFDiv(lValue, rValue, "divtmp");
			break;
		default:
			v = 0;
			break;
		}
	}

	if (vType.dstType != NO_TYPE && vType.dstType != vType.type) {
		v = typeCast(vType, v);
	}

	pending.insert(pending.end(), v);
}


void CodegenVisitor::visitUnaryExpNode(UnaryExpNode *node)
{
	Value *operandV = pending.back();
	pending.pop_back();

	if (operandV == 0) {
		pending.insert(pending.end(), 0);
		return;
	}

	Value *v;

	switch (node->op) {
	case '+':
		v = operandV;
		break;
	case '-':
		v = Builder.CreateNeg(operandV, "negtmp");
		break;
	case '&':
	default:
		v = 0;
		break;
	}

	pending.insert(pending.end(), v);
}


void CodegenVisitor::visitIdNode(IdNode *node)
{
	Value *vPtr = lookUp(*node->name);

	Value *v;

	// if this id is a function name
	if (vPtr == nullptr) {
		v = TheModule->getFunction(*node->name);
	}

	// if this id is an array
	else if (((AllocaInst*)vPtr)->getAllocatedType()->isArrayTy()) {
		v = vPtr;
	}

	// if this id is a struct
	else if (((AllocaInst*)vPtr)->getAllocatedType()->isStructTy()) {
		v = vPtr;
	}

	// else
	else {
		// if vPtr is a global contant, return its value directly
		if (vPtr->getValueID() == vPtr->GlobalVariableVal &&
				((GlobalVariable *)vPtr)->isConstant()) {
			v = ((GlobalVariable *)vPtr)->getInitializer();
		}
		else {
			v = Builder.CreateLoad(vPtr, node->name->c_str());
		}
	}

	ValueTypeS vType = node->valueTy;
	if (vType.dstType != NO_TYPE && vType.dstType != vType.type)
		v = typeCast(vType, v);

	pending.insert(pending.end(), v);
}


void CodegenVisitor::visitArrayItemNode(ArrayItemNode *node)
{
	Value *retV;
	int size = node->index->nodes.size();
	vector<Value*> indexVs = getValuesFromStack(size);

	std::vector<Value *> idxList;
	idxList.push_back(ConstantInt::get(getGlobalContext(), APInt(32, 0, true)));
	for (int i = 0; i < size; i++) {
		Value *sextIndex = Builder.CreateSExt(indexVs[i], Type::getInt64Ty(getGlobalContext()));
		idxList.push_back(sextIndex);
	}

	Value *arrayPtr = pending.back();
	pending.pop_back();

	Value *arrayItemPtr = Builder.CreateGEP(arrayPtr, idxList, "array_ptr");
	retV = Builder.CreateLoad(arrayItemPtr, "array_item");
	pending.insert(pending.end(), retV);
}


void CodegenVisitor::visitStructItemNode(StructItemNode *node)
{
	// struct.item
	if (!node->isPointer) {
		Value *structPtr = pending.back();
		pending.pop_back();

		std::string structName = *node->stru->valueTy.structName;

		std::map<std::string, int> &offsetMap = *structOffsetTable[structName];
		int offset = offsetMap[*node->itemName];
		std::vector<Value *> idxV;
		idxV.push_back(ConstantInt::get(getGlobalContext(), APInt(32, 0, true)));
		idxV.push_back(ConstantInt::get(getGlobalContext(), APInt(32, offset, true)));
		Value *structItemPtr = Builder.CreateGEP(structPtr, idxV, "struct_item");

		Value *retV = Builder.CreateLoad(structItemPtr, false);

		pending.insert(pending.end(), retV);
	}
	// structPtr->item
	else {

	}
}


void CodegenVisitor::visitFunCallNode(FunCallNode *node)
{
	// get args
	std::list<Node *> arguments;
	if (node->hasArgs)
		arguments = node->argv->nodes;

	std::vector<Value *> argsV = getValuesFromStack(arguments.size());

	// get callee
	Function *calleeF = (Function *)pending.back();
	pending.pop_back();

	Value *retV = Builder.CreateCall(calleeF, argsV);
	pending.insert(pending.end(), retV);
}


void CodegenVisitor::visitIdVarDefNode(IdVarDefNode *node)
{
	std::string *name = node->name;
	Type *type = getLLVMVarType(node->valueTy);
	// global variable
	if (Builder.GetInsertBlock() == nullptr) {
		GlobalVariable *gVar = new GlobalVariable(*TheModule, /* module */
				type, /* type */
				node->valueTy.isConstant, 	/* is constant ? */
				getLinkageTyp(node->valueTy), /* linkage */
				0,	/* initializer */
				name->c_str() /* name */);

		// initialization
		Value *val = 0;
		if (node->isAssigned) {
			val = pending.back();
			pending.pop_back();
			if (val == 0)
				return;

			gVar->setInitializer((Constant*)val);
		}
		else {
			gVar->setInitializer(Constant::getNullValue(type));
		}

		GloblalVariables[*name] = gVar;
	}
	// local variable
	else {
		std::map<std::string, AllocaInst *> &LocalVariables = *LocalTableStack[StackPtr-1];

		Function *currentFunc =
				Builder.GetInsertBlock()->getParent();
		IRBuilder<> TmpBuilder(&currentFunc->getEntryBlock(), currentFunc->getEntryBlock().begin());

		AllocaInst *variable =
				TmpBuilder.CreateAlloca(getLLVMVarType(node->valueTy), 0, name->c_str());

		Value *val = 0;
		if (node->isAssigned) {
			val = pending.back();
			pending.pop_back();
			if (val == 0)
				return;
			Builder.CreateStore(val, variable);
		}

		if (node->valueTy.isConstant) {
			std::map<std::string, AllocaInst *> &ConstLocalVariables = *ConstLocalTableStack[StackPtr-1];
			ConstLocalVariables[*name] = variable;
		}
		else {
			std::map<std::string, AllocaInst *> &LocalVariables = *LocalTableStack[StackPtr-1];
			LocalVariables[*name] = variable;
		}
	}

}


void CodegenVisitor::visitArrayVarDefNode(ArrayVarDefNode *node)
{
	std::string *name = node->name;
	ValueTypeS vType = node->valueTy;

	int valuesSize;
	if (node->isAssigned)
		valuesSize = node->values->nodes.size();
	else
		valuesSize = 0;

	// get the size of array
	int arraySize = vType.base[0];

	ArrayType* arrayType = (ArrayType*)getLLVMVarType(vType);

	// global variable
	if (Builder.GetInsertBlock() == nullptr) {

		// insert global variable
		GlobalVariable *gVar = new GlobalVariable(*TheModule, /* module */
						arrayType, 		/* type */
						node->valueTy.isConstant, 	/* is constant ? */
						getLinkageTyp(node->valueTy), /* linkage */
						0,	/* initializer */
						name->c_str() /* name */);

		// initialize
		std::vector<Constant *> arrayItems(arraySize);
		if (node->isAssigned) {
			std::vector<Value *> values = getValuesFromStack(valuesSize);
			for (int i = 0; i < arraySize; ++i) {
				if (i < valuesSize)
					arrayItems[i] = (Constant *)(values[i]);
				else
					arrayItems[i] = Constant::getNullValue(arrayType->getArrayElementType());
			}
		}
		else {
			for (int i = 0; i < arraySize; ++i) {
				arrayItems[i] = Constant::getNullValue(arrayType->getArrayElementType());
			}
		}

		Constant* constArray = ConstantArray::get(arrayType, arrayItems);
		gVar->setInitializer(constArray);

		GloblalVariables[*name] = gVar;
	}
	// local variable
	else {
		Function *currentFunc = Builder.GetInsertBlock()->getParent();
		IRBuilder<> TmpBuilder(&currentFunc->getEntryBlock(), currentFunc->getEntryBlock().begin());

		AllocaInst *arrayPtr = TmpBuilder.CreateAlloca(arrayType, 0, name->c_str());

		// initialize
		if (node->isAssigned) {
			std::vector<Value *> values = getValuesFromStack(valuesSize);
			for (int i = 0; i < arraySize; i++) {
				Value *v;
				if (i < valuesSize)
					v = values[i];
				else
					v = Constant::getNullValue(arrayType->getArrayElementType());

				std::vector<Value *> idxList;
				idxList.push_back(ConstantInt::get(getGlobalContext(), APInt(32, 0, true)));
				idxList.push_back(ConstantInt::get(getGlobalContext(), APInt(64, i, true)));

				Value *arrayItemPtr = Builder.CreateGEP(arrayPtr, idxList, "array_init_" + *name);
				Builder.CreateStore(v, arrayItemPtr);
			}
		}

		if (node->valueTy.isConstant) {
			std::map<std::string, AllocaInst *> &ConstLocalVariables = *ConstLocalTableStack[StackPtr-1];
			ConstLocalVariables[*name] = arrayPtr;
		}
		else {
			std::map<std::string, AllocaInst *> &LocalVariables = *LocalTableStack[StackPtr-1];
			LocalVariables[*name] = arrayPtr;
		}
	}
}


void CodegenVisitor::visitEmptyNode(EmptyNode *node)
{
	// empty
}


void CodegenVisitor::visitBlockNode(BlockNode *node)
{
	// exit current scope
	StackPtr--;
	delete LocalTableStack[StackPtr];
	delete ConstLocalTableStack[StackPtr];
}


void CodegenVisitor::visitVarDeclNode(VarDeclNode *node)
{
	// empty
}


void CodegenVisitor::visitAssignStmtNode(AssignStmtNode *node)
{
	node->exp->accept(*this);
	Value *expV = pending.back();
	pending.pop_back();
	if (expV == 0)
		return;

	ValueTypeS lvalTy = node->lval->valueTy;
	ValueTypeS expTy = node->exp->valueTy;

	switch (node->lval->type) {
	case ID_AST:
	{
		ValueTypeS vType = node->lval->valueTy;

		// if this id is a struct
		if (vType.type == STRUCT_TYPE) {
			// no time to support this...sad
			break;
		}

		// id is atom type
		IdNode *lval = dynamic_cast<IdNode *>(node->lval);
		Value *lvalV = lookUp(*(lval->name));
		Builder.CreateStore(expV, lvalV);
		break;
	}
	case ARRAY_ITEM_AST:
	{
		ArrayItemNode *lval = dynamic_cast<ArrayItemNode *>(node->lval);
		int size = lval->index->nodes.size();

		lval->array->accept(*this);
		lval->index->accept(*this);

		vector<Value*> indexVs = getValuesFromStack(size);
		std::vector<Value *> idxList;
		idxList.push_back(ConstantInt::get(getGlobalContext(), APInt(32, 0, true)));
		for (int i = 0; i < size; i++) {
			Value *sextIndex = Builder.CreateSExt(indexVs[i], Type::getInt64Ty(getGlobalContext()));
			idxList.push_back(sextIndex);
		}

		Value *arrayPtr = pending.back();
		pending.pop_back();

		Value *arrayItemPtr = Builder.CreateGEP(arrayPtr, idxList, "array_ptr");
		Builder.CreateStore(expV, arrayItemPtr);
		break;
	}
	case STRUCT_ITEM_AST:
	{
		StructItemNode *lval = dynamic_cast<StructItemNode*>(node->lval);
		if (!lval->isPointer) {
			lval->stru->accept(*this);
			Value *structPtr = pending.back();
			pending.pop_back();

			std::string structName = *lval->stru->valueTy.structName;

			std::map<std::string, int> &offsetMap = *structOffsetTable[structName];
			int offset = offsetMap[*lval->itemName];
			std::vector<Value *> idxV;
			idxV.push_back(ConstantInt::get(getGlobalContext(), APInt(32, 0, true)));
			idxV.push_back(ConstantInt::get(getGlobalContext(), APInt(32, offset, true)));
			Value *structItemPtr = Builder.CreateGEP(structPtr, idxV, "struct_item");

			Builder.CreateStore(expV, structItemPtr);
		}
		else {

		}
		break;
	}
	case UNARY_EXP_AST:
		break;
	default:
		break;
	}
}


void CodegenVisitor::visitFunCallStmtNode(FunCallStmtNode *node)
{
	pending.pop_back();
}


void CodegenVisitor::visitBlockStmtNode(BlockStmtNode *node)
{
	// empty
}


void CodegenVisitor::visitCondNode(CondNode *node)
{
	Value *lValue, *rValue;
	PHINode *pn;
	Function *theFunction;
	BasicBlock *beginBB, *shortBB, *longBB;

	char op = node->op;
	switch (op) {
	case OR_OP:
	case AND_OP:
		theFunction = Builder.GetInsertBlock()->getParent();
		beginBB = BasicBlock::Create(getGlobalContext(), "begin_cond", theFunction);
		longBB = BasicBlock::Create(getGlobalContext(), "long_path");
		shortBB = BasicBlock::Create(getGlobalContext(), "short_path");

		Builder.CreateBr(beginBB);
		// begin block
		Builder.SetInsertPoint(beginBB);

		node->lhs->accept(*this);
		lValue = pending.back();
		pending.pop_back();
		if (lValue == 0) {
			pending.insert(pending.end(), 0);
			return;
		}

		if (op == OR_OP)
			Builder.CreateCondBr(lValue, shortBB, longBB);
		else
			Builder.CreateCondBr(lValue, longBB, shortBB);
		beginBB = Builder.GetInsertBlock();

		// long block (no shortcut)
		theFunction->getBasicBlockList().push_back(longBB);
		Builder.SetInsertPoint(longBB);

		node->rhs->accept(*this);
		rValue = pending.back();
		pending.pop_back();
		if (rValue == 0) {
			pending.insert(pending.end(), 0);
			return;
		}

		if (op == OR_OP)
			rValue = Builder.CreateOr(lValue, rValue, "or_tmp");
		else
			rValue = Builder.CreateAnd(lValue, rValue, "and_tmp");
		Builder.CreateBr(shortBB);
		longBB = Builder.GetInsertBlock();

		// short block (shortcut)
		theFunction->getBasicBlockList().push_back(shortBB);
		Builder.SetInsertPoint(shortBB);
		pn = Builder.CreatePHI(Type::getInt1Ty(getGlobalContext()), 2, "cond_tmp");
		pn->addIncoming(lValue, beginBB);
		pn->addIncoming(rValue, longBB);
		pending.insert(pending.end(), pn);
		return;

	case NOT_OP:
		node->rhs->accept(*this);
		rValue = pending.back();
		pending.pop_back();
		if (rValue == 0) {
			pending.insert(pending.end(), 0);
			return;
		}
		pending.insert(pending.end(), Builder.CreateNot(rValue));
		return;

	default:
		break;
	}

	Value *retV;
	node->rhs->accept(*this);
	rValue = pending.back();
	pending.pop_back();
	node->lhs->accept(*this);
	lValue = pending.back();
	pending.pop_back();

	if (rValue == 0 || lValue == 0)
		retV = 0;
	if (node->lhs->valueTy.type == INT_TYPE) {
		switch (op) {
		case LT_OP:
			retV = Builder.CreateICmpSLT(lValue, rValue, "ltcmp");
			break;
		case GT_OP:
			retV = Builder.CreateICmpSGT(lValue, rValue, "rtcmp");
			break;
		case LTE_OP:
			retV = Builder.CreateICmpSLE(lValue, rValue, "lecmp");
			break;
		case GTE_OP:
			retV = Builder.CreateICmpSGE(lValue, rValue, "gecmp");
			break;
		case EQ_OP:
			retV = Builder.CreateICmpEQ(lValue, rValue, "eqcmp");
			break;
		case NEQ_OP:
			retV = Builder.CreateICmpNE(lValue, rValue, "necmp");
			break;
		default:
			retV = 0;
			break;
		}
	}
	else {
		switch (op) {
		case LT_OP:
			retV = Builder.CreateFCmpOLT(lValue, rValue, "ltcmp");
			break;
		case GT_OP:
			retV = Builder.CreateFCmpOGT(lValue, rValue, "rtcmp");
			break;
		case LTE_OP:
			retV = Builder.CreateFCmpOLE(lValue, rValue, "lecmp");
			break;
		case GTE_OP:
			retV = Builder.CreateFCmpOGE(lValue, rValue, "gecmp");
			break;
		case EQ_OP:
			retV = Builder.CreateFCmpOEQ(lValue, rValue, "eqcmp");
			break;
		case NEQ_OP:
			retV = Builder.CreateFCmpONE(lValue, rValue, "necmp");
			break;
		default:
			retV = 0;
			break;
		}
	}
	pending.insert(pending.end(), retV);
}


void CodegenVisitor::visitIfStmtNode(IfStmtNode *node)
{
	node->cond->accept(*this);
	Value *condV = pending.back();
	pending.pop_back();
	if (condV == 0)
		return;

	Function *theFunction = Builder.GetInsertBlock()->getParent();

	// Create blocks for the then and else cases.  Insert the 'then' block at the
	// end of the function.
	BasicBlock *thenBB =
	      BasicBlock::Create(getGlobalContext(), "then", theFunction);
	BasicBlock *elseBB = BasicBlock::Create(getGlobalContext(), "else");
	BasicBlock *mergeBB = BasicBlock::Create(getGlobalContext(), "ifcont");

	Builder.CreateCondBr(condV, thenBB, elseBB);

	// emit then block.
	Builder.SetInsertPoint(thenBB);
	node->then_stmt->accept(*this);
	Builder.CreateBr(mergeBB);

	// emit else block
	theFunction->getBasicBlockList().push_back(elseBB);
	Builder.SetInsertPoint(elseBB);
	if (node->hasElse)
		node->else_stmt->accept(*this);
	Builder.CreateBr(mergeBB);

	// emit merge block.
	theFunction->getBasicBlockList().push_back(mergeBB);
	Builder.SetInsertPoint(mergeBB);
}


void CodegenVisitor::visitWhileStmtNdoe(WhileStmtNode *node)
{
	Function *theFunction = Builder.GetInsertBlock()->getParent();

	BasicBlock *condBB = BasicBlock::Create(getGlobalContext(), "cond", theFunction);
	BasicBlock *bodyBB = BasicBlock::Create(getGlobalContext(), "body");
	BasicBlock *endBB = BasicBlock::Create(getGlobalContext(), "end");

	// Cond basic block
	Builder.CreateBr(condBB);
	Builder.SetInsertPoint(condBB);
	node->cond->accept(*this);
	Value *condV = pending.back();
	pending.pop_back();
	if (condV == 0)
		return;
	Builder.CreateCondBr(condV, bodyBB, endBB);

	// Body basic block
	theFunction->getBasicBlockList().push_back(bodyBB);
	Builder.SetInsertPoint(bodyBB);
	node->do_stmt->accept(*this);
	Builder.CreateBr(condBB);

	// End basic block
	theFunction->getBasicBlockList().push_back(endBB);
	Builder.SetInsertPoint(endBB);
}


void CodegenVisitor::visitReturnStmtNdoe(ReturnStmtNode *node)
{
	Value *retV = pending.back();
	pending.pop_back();

	Builder.CreateStore(retV, returnValue);
}


void CodegenVisitor::visitBreakStmtNode(BreakStmtNode *node)
{
	// bug exists, so keep empty
}


void CodegenVisitor::visitContinueStmtNode(ContinueStmtNode *node)
{
	// bug exists, so keep empty
}


void CodegenVisitor::visitFuncDeclNode(FuncDeclNode *node)
{
	std::string *name = node->name;
	std::list<Node *> argNames;
	if (node->hasArgs)
		argNames = node->valueTy.argv->nodes;


	FunctionType *FT = (FunctionType *)getLLVMVarType(node->valueTy);

	Function *F =
	      Function::Create(FT, getLinkageTyp(node->valueTy), name->c_str(), TheModule);

	// set names for all arguments
	Function::arg_iterator aIt = F->arg_begin();
	for (std::list<Node *>::iterator it = argNames.begin();
			it != argNames.end(); it++, aIt++) {
		IdNode *arg = (IdNode *)(*it);
		aIt->setName(*(arg->name));
	}
}


void CodegenVisitor::visitFuncDefNode(FuncDefNode *node)
{
	// enter new scope
	LocalTableStack[StackPtr] = new std::map<std::string, AllocaInst *>;
	ConstLocalTableStack[StackPtr] = new std::map<std::string, AllocaInst *>;
	StackPtr++;

	node->decl->accept(*this);
	Function *F = TheModule->getFunction(*(node->decl->name));
	if (F == 0)
		return;

	std::map<std::string, AllocaInst *> &LocalVariables = *LocalTableStack[StackPtr-1];

	// insert entry block
	BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", F);
	Builder.SetInsertPoint(BB);

	// create end function BB
	funcEndBB = BasicBlock::Create(getGlobalContext(), "end_function");

	// create an alloca for return value
	returnValue = Builder.CreateAlloca(Type::getInt32Ty(getGlobalContext()), 0, "return_value");

	// create an alloca for each argument
	for (Function::arg_iterator aIt = F->arg_begin(); aIt != F->arg_end(); aIt++) {
		AllocaInst *alloca = Builder.CreateAlloca(Type::getInt32Ty(getGlobalContext()), 0, aIt->getName());
		Builder.CreateStore(aIt, alloca);
		LocalVariables[aIt->getName()] = alloca;
	}


	node->block->accept(*this);

	Builder.CreateBr(funcEndBB);
	F->getBasicBlockList().push_back(funcEndBB);
	Builder.SetInsertPoint(funcEndBB);

	if (F->getReturnType() == Type::getVoidTy(getGlobalContext()))
		Builder.CreateRetVoid();
	else
		Builder.CreateRet(Builder.CreateLoad(returnValue));


    // validate the generated code, checking for consistency
	verifyFunction(*F);

	// optimize the function
	TheFPM->run(*F);

	Builder.ClearInsertionPoint();


	// exit current scope
	StackPtr--;
	delete LocalTableStack[StackPtr];
	delete ConstLocalTableStack[StackPtr];
}


void CodegenVisitor::visitStructDefNode(StructDefNode *node)
{
	std::string nameStr = *node->name;

	std::map<std::string, int> *structOffset = new std::map<std::string, int>;
	StructType *structType = StructType::create(getGlobalContext(), nameStr);
	std::vector<Type*> attrTypes;

	std::list<Node *> nodes = node->decls->nodes;
	int i = 0;
	for (std::list<Node*>::iterator it = nodes.begin();
			it != nodes.end(); ++it) {
		attrTypes.push_back(getLLVMVarType((*it)->valueTy));
		Node *defNode = dynamic_cast<VarDeclNode*>(*it)->defList->nodes.front();
		std::string attrName = *dynamic_cast<VarDefNode*>(defNode)->name;
		(*structOffset)[attrName] = i;
		++i;
	}

	structOffsetTable[nameStr] = structOffset;

	structType->setBody(attrTypes, false);
}


void CodegenVisitor::visitCompUnitNode(CompUnitNode *node)
{
	// empty
}


void CodegenVisitor::enterBlockNode(BlockNode *node)
{
	// enter new scope
	LocalTableStack[StackPtr] = new std::map<std::string, AllocaInst *>;
	ConstLocalTableStack[StackPtr] = new std::map<std::string, AllocaInst *>;
	StackPtr++;
}


void CodegenVisitor::enterIfStmtNode(IfStmtNode *node)
{
	// empty
}


void CodegenVisitor::enterWhileStmtNode(WhileStmtNode *node)
{
	// empty
}


void CodegenVisitor::enterFuncDefNode(FuncDefNode *node)
{
	//empty
}


void CodegenVisitor::enterStructDefNode(StructDefNode *node)
{
}

