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
	Value *v = nullptr;
	int sp = StackPtr;

	while (sp > 0) {
		std::map<std::string, AllocaInst *> &ConstLocalVariables = *ConstLocalTableStack[StackPtr-1];
		std::map<std::string, AllocaInst *> &LocalVariables = *LocalTableStack[StackPtr-1];
		if (ConstLocalVariables.find(*name) != ConstLocalVariables.end()) {
			v = ConstLocalVariables[*name];
			break;
		}
		else if (LocalVariables.find(*name) != LocalVariables.end()) {
			v = LocalVariables[*name];
			break;
		}
		else {
			v = nullptr;
			sp--;
		}
	}

	if (v == nullptr) {
		if (GloblalVariables.find(*name) != GloblalVariables.end())
			v = GloblalVariables[*name];
		else {
			// error, undeclared identifier
			Error err =
					msgFactory.newError(e_undeclared_identifier, loc->first_line, loc->first_column);
			errorFlag = true;
			msgFactory.showMsg(&err);
			return 0;
		}
	}

	return Builder.CreateLoad(v, name->c_str());
}


Value *IdConstDefNode::codegen()
{
	// global variable
	if (Builder.GetInsertBlock() == nullptr) {
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

		if (val->getValueID() == val->ConstantIntVal)
			gVar->setInitializer((ConstantInt*)val);
		else {
			// error, try to initialize a global variable with non-constant value
			gVar->setInitializer(ConstantInt::get(getGlobalContext(), APInt(32, 0, true)));
			errorFlag = true;
			Error err =
					msgFactory.newError(e_global_init_not_constant, loc->first_line, loc->first_column);
			msgFactory.showMsg(&err);
		}

		GloblalVariables[*name] = gVar;
	}
	// local variable
	else {
		std::map<std::string, AllocaInst *> &ConstLocalVariables = *ConstLocalTableStack[StackPtr-1];
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


Value *IdVarDefNode::codegen()
{
	// global variable
	if (Builder.GetInsertBlock() == nullptr) {
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
				Error err =
						msgFactory.newError(e_global_init_not_constant, loc->first_line, loc->first_column);
				msgFactory.showMsg(&err);
			}
		}
		else {
			gVar->setInitializer(ConstantInt::get(getGlobalContext(), APInt(32, 0, true)));
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


Function *FuncDefNode::codegen()
{
	// enter new scope
	LocalTableStack[StackPtr] = new std::map<std::string, AllocaInst *>;
	ConstLocalTableStack[StackPtr] = new std::map<std::string, AllocaInst *>;
	StackPtr++;

	std::vector<Type *> Ints(0);
	FunctionType *FT =
			FunctionType::get(Type::getVoidTy(getGlobalContext()), Ints, false);
	Function *F =
	      Function::Create(FT, Function::ExternalLinkage, name->c_str(), TheModule);

	// If F conflicted, there was already something named 'Name'.
	if (F->getName() != *name) {
		F->eraseFromParent();
		errorFlag = true;
		Error err =
				msgFactory.newError(e_redefinition_of_function, loc->first_line, loc->first_column);
		msgFactory.showMsg(&err);
	}

	BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", F);
	Builder.SetInsertPoint(BB);

	block->codegen();

	verifyFunction(*F);

	// exit current scope
	StackPtr--;
	delete LocalTableStack[StackPtr];
	delete ConstLocalTableStack[StackPtr];

	Builder.ClearInsertionPoint();

	return F;
}


Value *CompUnitNode::codegen()
{
	for (std::list<Node*>::iterator it = nodes.begin(); it != nodes.end(); it++) {
			(*it)->codegen();
	}
	return 0;
}








