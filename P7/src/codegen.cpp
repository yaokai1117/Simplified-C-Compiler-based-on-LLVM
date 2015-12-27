#include "llvm/Analysis/Passes.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/PassManager.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"
#include <cctype>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "node.h"
#include "msgfactory.h"

using namespace llvm;

extern Module *TheModule;
IRBuilder<> Builder(getGlobalContext());

extern MsgFactory msgFactory;
extern bool errorFlag;


static std::map<std::string, AllocaInst *> *ConstLocalTableStack[32];
static std::map<std::string, AllocaInst *> *LocalTableStack[32];
static std::map<std::string, GlobalVariable *> GloblalVariables;

static int StackPtr = 0;


static Value *lookUp(std::string nameStr, bool &isConst)
{
	Value *retV = nullptr;
	int sp = StackPtr;
	while (sp > 0) {
		std::map<std::string, AllocaInst *> &ConstLocalVariables = *ConstLocalTableStack[sp-1];
		std::map<std::string, AllocaInst *> &LocalVariables = *LocalTableStack[sp-1];
		if (ConstLocalVariables.find(nameStr) != ConstLocalVariables.end()) {
			retV = ConstLocalVariables[nameStr];
			isConst = true;
			break;
		}
		else if (LocalVariables.find(nameStr) != LocalVariables.end()) {
			retV = LocalVariables[nameStr];
			isConst = false;
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
			isConst = ((GlobalVariable *)retV)->isConstant();
		}
		else
			return 0;
	}
	return retV;
}


Value *NodeList::codegen()
{
	for (std::list<Node*>::iterator it = nodes.begin(); it != nodes.end(); it++) {
		(*it)->codegen();
	}
	return 0;
}


Value *NumNode::codegen()
{
	return ConstantInt::get(getGlobalContext(), APInt(32, val, true));
}


Value *BinaryExpNode::codegen()
{
	Value *lValue = lhs->codegen();
	Value *rValue = rhs->codegen();
	if (lValue == 0 || rValue == 0)
		return 0;
	switch (op) {
	case '+':
		return Builder.CreateAdd(lValue, rValue, "addtmp");
	case '-':
		return Builder.CreateSub(lValue, rValue, "subtmp");
	case '*':
		return Builder.CreateMul(lValue, rValue, "multmp");
	case '/':
		return Builder.CreateSDiv(lValue, rValue, "divtmp");
	case '%':
		return Builder.CreateSRem(lValue, rValue, "modtmp");
	default:
		break;
	}
	return 0;
}


Value *UnaryExpNode::codegen()
{
	Value *operandV = operand->codegen();
	if (operandV == 0)
		return 0;

	switch (op) {
	case '+':
		return operandV;
	case '-':
		return Builder.CreateNeg(operandV, "negtmp");
	default:
		break;
	}
	return 0;
}


Value *IdNode::codegen()
{
	bool isConst = false;
	Value *v = lookUp(*name, isConst);
	if (v == 0) {
		// error, undeclared identifier
		msgFactory.newError(e_undeclared_identifier, loc->first_line, loc->first_column);
		errorFlag = true;
		return 0;
	}

	// if v is a global contant, return its value directly
	if (v->getValueID() == v->GlobalVariableVal &&
			((GlobalVariable *)v)->isConstant())
		return ((GlobalVariable *)v)->getInitializer();

	return Builder.CreateLoad(v, name->c_str());
}


Value *ArrayItemNode::codegen()
{
	bool isConst = false;
	Value *arrayPtr = lookUp(*name, isConst);
	if (arrayPtr == 0) {
		// error, undeclared identifier
		msgFactory.newError(e_undeclared_identifier, loc->first_line, loc->first_column);
		errorFlag = true;
		return 0;
	}

	Value *indexV = index->codegen();
	if (indexV == 0)
		return 0;

	std::vector<Value *> idxList;
	idxList.push_back(ConstantInt::get(getGlobalContext(), APInt(32, 0, true)));
	Value *sextIndex = Builder.CreateSExt(indexV, Type::getInt64Ty(getGlobalContext()));
	idxList.push_back(sextIndex);

	Value *arrayItemPtr = Builder.CreateGEP(arrayPtr, idxList, "array_ptr");

	return Builder.CreateLoad(arrayItemPtr, "array_item_" + *name);
}


Value *IdConstDefNode::codegen()
{
	// global variable
	if (Builder.GetInsertBlock() == nullptr) {
		// check if this identifier was already defined
		if (GloblalVariables.find(*name) != GloblalVariables.end()) {
			msgFactory.newError(e_redefinition_of_identifier, loc->first_line, loc->first_column);
			return 0;
		}

		// insert global variable
		GlobalVariable *gVar = new GlobalVariable(*TheModule, /* module */
						Type::getInt32Ty(getGlobalContext()), /* type */
						true, 	/* is constant ? */
						GlobalValue::ExternalLinkage, /* linkage */
						0,	/* initializer */
						name->c_str() /* name */);
		gVar->setAlignment(4);

		Value *val = value->codegen();
		if (val == 0)
			return 0;

		// initialization
		if (val->getValueID() == val->ConstantIntVal)
			gVar->setInitializer((ConstantInt*)val);
		else {
			// error, try to initialize a global variable with non-constant value
			gVar->setInitializer(ConstantInt::get(getGlobalContext(), APInt(32, 0, true)));
			errorFlag = true;
			msgFactory.newError(e_global_init_not_constant, loc->first_line, loc->first_column);
			return 0;
		}

		GloblalVariables[*name] = gVar;
	}
	// local variable
	else {
		// check if this identifier was already defined
		std::map<std::string, AllocaInst *> &ConstLocalVariables = *ConstLocalTableStack[StackPtr-1];
		if (ConstLocalVariables.find(*name) != ConstLocalVariables.end()) {
			msgFactory.newError(e_redefinition_of_identifier, loc->first_line, loc->first_column);
			return 0;
		}

		// insert local variable
		Function *currentFunc = Builder.GetInsertBlock()->getParent();
		IRBuilder<> TmpBuilder(&currentFunc->getEntryBlock(), currentFunc->getEntryBlock().begin());

		AllocaInst *variable =
				TmpBuilder.CreateAlloca(Type::getInt32Ty(getGlobalContext()), 0, name->c_str());

		Value *val = value->codegen();
		if (val == 0)
			return 0;

		Builder.CreateStore(val, variable);

		ConstLocalVariables[*name] = variable;
	}

	return 0;
}


Value *ArrayConstDefNode::codegen()
{
	int arraySize = 0;
	std::list<Node *> exps = values->nodes;

	// get the size of array
	if (hasSize) {
		Value *sizeV = size->codegen();
		if (sizeV == 0)
			return 0;

		if (sizeV->getValueID() == sizeV->ConstantIntVal) {
			arraySize = (int)*((ConstantInt *)sizeV)->getValue().getRawData();
		}
		else {
			msgFactory.newError(e_global_init_not_constant, loc->first_line, loc->first_column);
			return 0;
		}
	}
	else {
		arraySize = exps.size();
	}

	ArrayType* arrayType = ArrayType::get(IntegerType::get(TheModule->getContext(), 32), arraySize);

	// global variable
	if (Builder.GetInsertBlock() == nullptr) {
		// check if this identifier was already defined
		if (GloblalVariables.find(*name) != GloblalVariables.end()) {
			msgFactory.newError(e_redefinition_of_identifier, loc->first_line, loc->first_column);
			return 0;
		}
		// insert global variable
		GlobalVariable *gVar = new GlobalVariable(*TheModule, /* module */
						arrayType, /* type */
						true, 	/* is constant ? */
						GlobalValue::ExternalLinkage, /* linkage */
						0,	/* initializer */
						name->c_str() /* name */);
		gVar->setAlignment(4);

		// initialize
		std::vector<Constant *> arrayItems;
		std::list<Node *>::iterator expIt = exps.begin();
		for (int i = 0; i < arraySize; i++) {
			Value *expV;
			if (i < exps.size()) {
				expV = (*expIt)->codegen();
				expIt++;
				if (expV == 0)
					return 0;
			}
			else
				expV = ConstantInt::get(getGlobalContext(), APInt(32, 0, true));

			if (expV->getValueID() == expV->ConstantIntVal) {
				arrayItems.push_back((Constant *)expV);
			}
			else {
				msgFactory.newError(e_global_init_not_constant, loc->first_line, loc->first_column);
				return 0;
			}
		}
		Constant* constArray = ConstantArray::get(arrayType, arrayItems);
		gVar->setInitializer(constArray);

		GloblalVariables[*name] = gVar;
	}
	// local variable
	else {
		// check if this identifier was already defined
		std::map<std::string, AllocaInst *> &ConstLocalVariables = *ConstLocalTableStack[StackPtr-1];
		if (ConstLocalVariables.find(*name) != ConstLocalVariables.end()) {
			msgFactory.newError(e_redefinition_of_identifier, loc->first_line, loc->first_column);
			return 0;
		}
		// insert local variable
		Function *currentFunc = Builder.GetInsertBlock()->getParent();
		IRBuilder<> TmpBuilder(&currentFunc->getEntryBlock(), currentFunc->getEntryBlock().begin());

		AllocaInst *arrayPtr = TmpBuilder.CreateAlloca(arrayType, 0, name->c_str());

		// initialize
		std::list<Node *>::iterator expIt = exps.begin();
		for (int i = 0; i < arraySize; i++) {
			Value *expV;
			if (i < exps.size()) {
				expV = (*expIt)->codegen();
				expIt++;
				if (expV == 0)
					return 0;
			}
			else
				expV = ConstantInt::get(getGlobalContext(), APInt(32, 0, true));

			std::vector<Value *> idxList;
			idxList.push_back(ConstantInt::get(getGlobalContext(), APInt(32, 0, true)));
			idxList.push_back(ConstantInt::get(getGlobalContext(), APInt(64, i, true)));

			Value *arrayItemPtr = Builder.CreateGEP(arrayPtr, idxList, "array_init_" + *name);
			Builder.CreateStore(expV, arrayItemPtr);
		}
		ConstLocalVariables[*name] = arrayPtr;
	}
	return 0;
}


Value *IdVarDefNode::codegen()
{
	// global variable
	if (Builder.GetInsertBlock() == nullptr) {
		// check if this identifier was already defined
		if (GloblalVariables.find(*name) != GloblalVariables.end()) {
			msgFactory.newError(e_redefinition_of_identifier, loc->first_line, loc->first_column);
			return 0;
		}

		GlobalVariable *gVar = new GlobalVariable(*TheModule, /* module */
				Type::getInt32Ty(getGlobalContext()), /* type */
				false, 	/* is constant ? */
				GlobalValue::ExternalLinkage, /* linkage */
				0,	/* initializer */
				name->c_str() /* name */);
		gVar->setAlignment(4);

		Value *val = 0;
		if (isAssigned) {
			val = value->codegen();
			if (val == 0)
				return 0;

			if (val->getValueID() == val->ConstantIntVal)
				gVar->setInitializer((ConstantInt*)val);
			else {
				// error, try to initialize a global variable with non-constant value
				gVar->setInitializer(ConstantInt::get(getGlobalContext(), APInt(32, 0, true)));
				errorFlag = true;
				msgFactory.newError(e_global_init_not_constant, loc->first_line, loc->first_column);
				return 0;
			}
		}
		else {
			gVar->setInitializer(ConstantInt::get(getGlobalContext(), APInt(32, 0, true)));
		}

		GloblalVariables[*name] = gVar;
	}
	// local variable
	else {
		// check if this identifier was already defined
		std::map<std::string, AllocaInst *> &LocalVariables = *LocalTableStack[StackPtr-1];
		if (LocalVariables.find(*name) != LocalVariables.end()) {
			msgFactory.newError(e_redefinition_of_identifier, loc->first_line, loc->first_column);
			return 0;
		}

		Function *currentFunc =
				Builder.GetInsertBlock()->getParent();
		IRBuilder<> TmpBuilder(&currentFunc->getEntryBlock(), currentFunc->getEntryBlock().begin());

		AllocaInst *variable =
				TmpBuilder.CreateAlloca(Type::getInt32Ty(getGlobalContext()), 0, name->c_str());

		Value *val = 0;
		if (isAssigned) {
			val = value->codegen();
			if (val == 0)
				return 0;
			Builder.CreateStore(val, variable);
		}

		LocalVariables[*name] = variable;
	}

	return 0;
}


Value *ArrayVarDefNode::codegen()
{
	int arraySize = 0;

	// get the size of array
	if (hasSize) {
		Value *sizeV = size->codegen();
		if (sizeV == 0)
			return 0;

		if (sizeV->getValueID() == sizeV->ConstantIntVal) {
			arraySize = (int)*((ConstantInt *)sizeV)->getValue().getRawData();
		}
		else {
			msgFactory.newError(e_global_init_not_constant, loc->first_line, loc->first_column);
			return 0;
		}
	}
	else
		arraySize = values->nodes.size();


	ArrayType* arrayType = ArrayType::get(IntegerType::get(TheModule->getContext(), 32), arraySize);

	// global variable
	if (Builder.GetInsertBlock() == nullptr) {
		// check if this identifier was already defined
		if (GloblalVariables.find(*name) != GloblalVariables.end()) {
			msgFactory.newError(e_redefinition_of_identifier, loc->first_line, loc->first_column);
			return 0;
		}

		// insert global variable
		GlobalVariable *gVar = new GlobalVariable(*TheModule, /* module */
						arrayType, /* type */
						false, 	/* is constant ? */
						GlobalValue::ExternalLinkage, /* linkage */
						0,	/* initializer */
						name->c_str() /* name */);
		gVar->setAlignment(4);

		// initialize
		std::vector<Constant *> arrayItems;
		if (isAssigned) {
			std::list<Node *> exps = values->nodes;
			std::list<Node *>::iterator expIt = exps.begin();
			for (int i = 0; i < arraySize; i++) {
				Value *expV;
				if (i < exps.size()) {
					expV = (*expIt)->codegen();
					expIt++;
					if (expV == 0)
						return 0;
				}
				else
					expV = ConstantInt::get(getGlobalContext(), APInt(32, 0, true));

				if (expV->getValueID() == expV->ConstantIntVal) {
					arrayItems.push_back((Constant *)expV);
				}
				else {
					msgFactory.newError(e_global_init_not_constant, loc->first_line, loc->first_column);
					return 0;
				}
			}
		}
		else {
			arrayItems = std::vector<Constant *>(arraySize,
					ConstantInt::get(getGlobalContext(), APInt(32, 0, true)));
		}

		Constant* constArray = ConstantArray::get(arrayType, arrayItems);
		gVar->setInitializer(constArray);

		GloblalVariables[*name] = gVar;
	}
	// local variable
	else {
		// check if this identifier was already defined
		std::map<std::string, AllocaInst *> &LocalVariables = *LocalTableStack[StackPtr-1];
		if (LocalVariables.find(*name) != LocalVariables.end()) {
			msgFactory.newError(e_redefinition_of_identifier, loc->first_line, loc->first_column);
			return 0;
		}

		// insert local variable
		Function *currentFunc = Builder.GetInsertBlock()->getParent();
		IRBuilder<> TmpBuilder(&currentFunc->getEntryBlock(), currentFunc->getEntryBlock().begin());

		AllocaInst *arrayPtr = TmpBuilder.CreateAlloca(arrayType, 0, name->c_str());

		// initialize
		if (isAssigned) {
			std::list<Node *> exps = values->nodes;
			std::list<Node *>::iterator expIt = exps.begin();
			for (int i = 0; i < arraySize; i++) {
				Value *expV;
				if (i < exps.size()) {
					expV = (*expIt)->codegen();
					expIt++;
					if (expV == 0)
						return 0;
				}
				else
					expV = ConstantInt::get(getGlobalContext(), APInt(32, 0, true));

				std::vector<Value *> idxList;
				idxList.push_back(ConstantInt::get(getGlobalContext(), APInt(32, 0, true)));
				idxList.push_back(ConstantInt::get(getGlobalContext(), APInt(64, i, true)));

				Value *arrayItemPtr = Builder.CreateGEP(arrayPtr, idxList, "array_init_" + *name);
				Builder.CreateStore(expV, arrayItemPtr);
			}
		}

		LocalVariables[*name] = arrayPtr;
	}
	return 0;
}


Value *VarDeclNode::codegen()
{
	return defList->codegen();
}


Value *ConstDeclNode::codegen()
{
	return defList->codegen();
}


Value *BlockNode::codegen()
{
	return blockItems->codegen();
}


Value *AssignStmtNode::codegen()
{
	bool isConst = false;
	Value *expV = exp->codegen();
	if (expV == 0)
		return 0;

	if (lval->type == ID_AST) {
		std::string nameStr(*(dynamic_cast<IdNode *>(lval)->name));
		Value *lvalV = lookUp(nameStr, isConst);
		if (isConst) {
			msgFactory.newError(e_assign_to_constant, loc->first_line, loc->first_column);
			errorFlag = true;
			return 0;
		}
		if (lvalV == 0) {
			// error, undeclared identifier
			msgFactory.newError(e_undeclared_identifier, loc->first_line, loc->first_column);
			errorFlag = true;
			return 0;
		}
		Builder.CreateStore(expV, lvalV);
	}
	else {
		ArrayItemNode *arrayItemNode = dynamic_cast<ArrayItemNode *>(lval);

		Value *arrayPtr = lookUp(*(arrayItemNode->name), isConst);
		if (isConst) {
			msgFactory.newError(e_assign_to_constant, loc->first_line, loc->first_column);
			errorFlag = true;
			return 0;
		}
		if (arrayPtr == 0) {
			// error, undeclared identifier
			msgFactory.newError(e_undeclared_identifier, loc->first_line, loc->first_column);
			errorFlag = true;
			return 0;
		}

		Value *indexV = arrayItemNode->index->codegen();
		if (indexV == 0)
			return 0;

		std::vector<Value *> idxList;
		idxList.push_back(ConstantInt::get(getGlobalContext(), APInt(32, 0, true)));
		Value *sextIndex = Builder.CreateSExt(indexV, Type::getInt64Ty(getGlobalContext()));
		idxList.push_back(sextIndex);

		Value *arrayItemPtr = Builder.CreateGEP(arrayPtr, idxList, "array_ptr");

		Builder.CreateStore(expV, arrayItemPtr);
	}

	return 0;
}


Value *FunCallStmtNode::codegen()
{
	// Look up the name in the global module table.
	Function *calleeF = TheModule->getFunction(*name);
	if (calleeF == 0) {
		errorFlag = true;
		msgFactory.newError(e_unknown_function, loc->first_line, loc->first_column);
		return 0;
	}

	std::list<Node *> arguments;
	if (hasArgs)
		arguments = argv->nodes;

	// check for argument mismatch error
	if (arguments.size() != calleeF->arg_size()) {
		errorFlag = true;
		msgFactory.newError(e_argument_unmatch, loc->first_line, loc->first_column);
		return 0;
	}

	std::vector<Value *> argsV;
	for (std::list<Node *>::iterator it = arguments.begin();
			it != arguments.end(); it++) {
		argsV.push_back((*it)->codegen());
		if (argsV.back() == 0)
			return 0;
	}

	return Builder.CreateCall(calleeF, argsV);

}


Value *BlockStmtNode::codegen()
{
	return block->codegen();
}


Value *CondNode::codegen()
{
	Value *rValue = rhs->codegen();
	Value *lValue = lhs->codegen();
	if (rValue == 0 || lValue == 0)
		return 0;
	switch (op) {
	case LT_OP:
		return Builder.CreateICmpSLT(lValue, rValue, "ltcmp");
	case GT_OP:
		return Builder.CreateICmpSGT(lValue, rValue, "rtcmp");
	case LTE_OP:
		return Builder.CreateICmpSLE(lValue, rValue, "lecmp");
	case GTE_OP:
		return Builder.CreateICmpSGE(lValue, rValue, "gecmp");
	case EQ_OP:
		return Builder.CreateICmpEQ(lValue, rValue, "eqcmp");
	case NEQ_OP:
		return Builder.CreateICmpNE(lValue, rValue, "necmp");
	default:
		break;
	}
	return 0;
}


Value *IfStmtNode::codegen()
{
	// enter new scope
	LocalTableStack[StackPtr] = new std::map<std::string, AllocaInst *>;
	ConstLocalTableStack[StackPtr] = new std::map<std::string, AllocaInst *>;
	StackPtr++;

	Value *condV = cond->codegen();
	if (condV == 0)
		return 0;

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
	then_stmt->codegen();
	Builder.CreateBr(mergeBB);

	// emit else block
	theFunction->getBasicBlockList().push_back(elseBB);
	Builder.SetInsertPoint(elseBB);
	if (hasElse)
		else_stmt->codegen();
	Builder.CreateBr(mergeBB);

	// emit merge block.
	theFunction->getBasicBlockList().push_back(mergeBB);
	Builder.SetInsertPoint(mergeBB);

	// exit current scope
	StackPtr--;
	delete LocalTableStack[StackPtr];
	delete ConstLocalTableStack[StackPtr];

	return 0;
}


Value *WhileStmtNode::codegen()
{
	// enter new scope
	LocalTableStack[StackPtr] = new std::map<std::string, AllocaInst *>;
	ConstLocalTableStack[StackPtr] = new std::map<std::string, AllocaInst *>;
	StackPtr++;

	Function *theFunction = Builder.GetInsertBlock()->getParent();

	BasicBlock *condBB = BasicBlock::Create(getGlobalContext(), "cond", theFunction);
	BasicBlock *bodyBB = BasicBlock::Create(getGlobalContext(), "body");
	BasicBlock *endBB = BasicBlock::Create(getGlobalContext(), "end");

	// Cond basic block
	Builder.CreateBr(condBB);
	Builder.SetInsertPoint(condBB);
	Value *condV = cond->codegen();
	if (condV == 0)
	    return 0;
	Builder.CreateCondBr(condV, bodyBB, endBB);

	// Body basic block
	theFunction->getBasicBlockList().push_back(bodyBB);
	Builder.SetInsertPoint(bodyBB);
	do_stmt->codegen();
	Builder.CreateBr(condBB);

	// End basic block
	theFunction->getBasicBlockList().push_back(endBB);
	Builder.SetInsertPoint(endBB);

	// exit current scope
	StackPtr--;
	delete LocalTableStack[StackPtr];
	delete ConstLocalTableStack[StackPtr];

	return 0;
}


Function *FuncDefNode::codegen()
{
	// enter new scope
	LocalTableStack[StackPtr] = new std::map<std::string, AllocaInst *>;
	ConstLocalTableStack[StackPtr] = new std::map<std::string, AllocaInst *>;
	StackPtr++;

	std::map<std::string, AllocaInst *> &LocalVariables = *LocalTableStack[StackPtr-1];

	std::list<Node *> argNames;
	if (hasArgs)
		argNames = argv->nodes;

	std::vector<Type *> Ints(argNames.size(), Type::getInt32Ty(getGlobalContext()));
	FunctionType *FT =
			FunctionType::get(Type::getVoidTy(getGlobalContext()), Ints, false);
	Function *F =
	      Function::Create(FT, Function::ExternalLinkage, name->c_str(), TheModule);


	// sf F conflicted, there was already something named 'Name'.
	if (F->getName() != *name) {
		F->eraseFromParent();
		errorFlag = true;
		msgFactory.newError(e_redefinition_of_function, loc->first_line, loc->first_column);
		return 0;
	}

	// set names for all arguments
	Function::arg_iterator aIt = F->arg_begin();
	for (std::list<Node *>::iterator it = argNames.begin();
			it != argNames.end(); it++, aIt++) {
		IdNode *arg = (IdNode *)(*it);
		aIt->setName(*(arg->name));
	}

	// insert entry block
	BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", F);
	Builder.SetInsertPoint(BB);

	// create an alloca for each argument
	for (aIt = F->arg_begin(); aIt != F->arg_end(); aIt++) {
		AllocaInst *alloca = Builder.CreateAlloca(Type::getInt32Ty(getGlobalContext()), 0, aIt->getName());
		Builder.CreateStore(aIt, alloca);
		LocalVariables[aIt->getName()] = alloca;
	}


	block->codegen();


    Builder.CreateRetVoid();

	verifyFunction(*F);

	// exit current scope
	StackPtr--;
	delete LocalTableStack[StackPtr];
	delete ConstLocalTableStack[StackPtr];

	Builder.ClearInsertionPoint();

	return 0;
}


Value *CompUnitNode::codegen()
{

	// debug
    GlobalVariable *Output =
    		new GlobalVariable(*TheModule, IntegerType::get(getGlobalContext(), 32),
    				false, GlobalVariable::ExternalLinkage, 0, "Output");
    GloblalVariables["Output"] = Output;

    std::vector<Type *> func_print_args;
    FunctionType *func_print_type =
    		FunctionType::get(Type::getVoidTy(getGlobalContext()), func_print_args, TheModule);
    Function *func_print =
    		Function::Create(func_print_type, GlobalVariable::ExternalLinkage, "print", TheModule);
	// end debug

	for (std::list<Node*>::iterator it = nodes.begin(); it != nodes.end(); it++) {
			(*it)->codegen();
	}
	return 0;
}








