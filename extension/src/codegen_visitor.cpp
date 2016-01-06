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


static Module *TheModule;
static ExecutionEngine *TheExecutionEngine;
static FunctionPassManager *TheFPM;
static IRBuilder<> Builder(llvm::getGlobalContext());

Value *CodegenVisitor::lookUp(std::string nameStr, bool &isConst)
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

    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();

    LLVMContext &Context = getGlobalContext();
    std::unique_ptr<Module> Owner = make_unique<Module>("Yao Kai's compiler !!!", Context);
    TheModule = Owner.get();

    std::string ErrStr;
    TheExecutionEngine =
    		EngineBuilder(std::move(Owner))
              .setErrorStr(&ErrStr)
              .setMCJITMemoryManager(llvm::make_unique<llvm::SectionMemoryManager>())
              .create();
    if (!TheExecutionEngine) {
        fprintf(stdout, "Could not create ExecutionEngine: %s\n", ErrStr.c_str());
        exit(1);
    }

    FunctionPassManager OurFPM(TheModule);

    // Set up the optimizer pipeline.  Start with registering info about how the
    // target lays out data structures.
    TheModule->setDataLayout(TheExecutionEngine->getDataLayout());
    OurFPM.add(new llvm::DataLayoutPass());
    // Provide basic AliasAnalysis support for GVN.
    OurFPM.add(llvm::createBasicAliasAnalysisPass());
    // Promote allocas to registers.
    OurFPM.add(llvm::createPromoteMemoryToRegisterPass());
    // Do simple "peephole" optimizations and bit-twiddling optzns.
    OurFPM.add(llvm::createInstructionCombiningPass());
    // Reassociate expressions.
    OurFPM.add(llvm::createReassociatePass());
    // Eliminate Common SubExpressions.
    OurFPM.add(llvm::createGVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    OurFPM.add(llvm::createCFGSimplificationPass());

    OurFPM.doInitialization();

    // Set the global so the code gen can use this.
    TheFPM = &OurFPM;

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
	pending.insert(pending.end(), v);
}


void CodegenVisitor::visitFNumNode(FNumNode *node)
{
	Value *v = ConstantFP::get(getGlobalContext(), APFloat(node->fval));
	pending.insert(pending.end(), v);
}


void CodegenVisitor::visitCharNode(CharNode *node)
{
	Value *v = ConstantInt::get(getGlobalContext(), APInt(8, (int)(node->cval), true));
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
	default:
		v = 0;
		break;
	}

	pending.insert(pending.end(), v);
}


void CodegenVisitor::visitIdNode(IdNode *node)
{
	bool isConst = false;
	Value *vPtr = lookUp(*(node->name), isConst);

	Value *v;

	// if vPtr is a global contant, return its value directly
	if (vPtr->getValueID() == vPtr->GlobalVariableVal &&
			((GlobalVariable *)vPtr)->isConstant()) {
		v = ((GlobalVariable *)vPtr)->getInitializer();
	}
	else {
		v = Builder.CreateLoad(vPtr, node->name->c_str());
	}

	pending.insert(pending.end(), v);
}


void CodegenVisitor::visitArrayItemNdoe(ArrayItemNode *node)
{
	bool isConst = false;
	Value *arrayPtr = lookUp(*(node->name), isConst);

	Value *indexV = pending.back();
	pending.pop_back();
	if (indexV == 0) {
		pending.insert(pending.end(), 0);
		return;
	}

	std::vector<Value *> idxList;
	idxList.push_back(ConstantInt::get(getGlobalContext(), APInt(32, 0, true)));
	Value *sextIndex = Builder.CreateSExt(indexV, Type::getInt64Ty(getGlobalContext()));
	idxList.push_back(sextIndex);

	Value *arrayItemPtr = Builder.CreateGEP(arrayPtr, idxList, "array_ptr");
	Value *retV = Builder.CreateLoad(arrayItemPtr, "array_item_" + *(node->name));

	pending.insert(pending.end(), retV);
}


void CodegenVisitor::visitIdVarDefNode(IdVarDefNode *node)
{
	std::string *name = node->name;
	// global variable
	if (Builder.GetInsertBlock() == nullptr) {
		GlobalVariable *gVar = new GlobalVariable(*TheModule, /* module */
				Type::getInt32Ty(getGlobalContext()), /* type */
				node->isConstant, 	/* is constant ? */
				GlobalValue::ExternalLinkage, /* linkage */
				0,	/* initializer */
				name->c_str() /* name */);
		gVar->setAlignment(4);

		Value *val = 0;
		if (node->isAssigned) {
			val = pending.back();
			pending.pop_back();
			if (val == 0)
				return;

			gVar->setInitializer((ConstantInt*)val);
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
		if (node->isAssigned) {
			val = pending.back();
			pending.pop_back();
			if (val == 0)
				return;
			Builder.CreateStore(val, variable);
		}

		if (node->isConstant) {
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

	int valuesSize;
	if (node->isAssigned)
		valuesSize = node->values->nodes.size();
	else
		valuesSize = 0;

	// get the size of array
	int arraySize = 0;
	if (node->hasSize) {
		Value *sizeV = pending[pending.size() - valuesSize - 1];

		if (sizeV == 0)
			return;

		arraySize = (int)*((ConstantInt *)sizeV)->getValue().getRawData();
	}
	else
		arraySize = valuesSize;


	ArrayType* arrayType = ArrayType::get(IntegerType::get(TheModule->getContext(), 32), arraySize);

	// global variable
	if (Builder.GetInsertBlock() == nullptr) {

		// insert global variable
		GlobalVariable *gVar = new GlobalVariable(*TheModule, /* module */
						arrayType, 		/* type */
						node->isConstant, 	/* is constant ? */
						GlobalValue::ExternalLinkage, /* linkage */
						0,	/* initializer */
						name->c_str() /* name */);
		gVar->setAlignment(4);

		// initialize
		std::vector<Constant *> arrayItems(arraySize);
		if (node->isAssigned) {
			std::vector<Value *> values = getValuesFromStack(valuesSize);
			for (int i = 0; i < arraySize; i++) {
				if (i < valuesSize) {
					arrayItems[i] = (Constant *)(values[i]);
				}
				else
					arrayItems[i] = ConstantInt::get(getGlobalContext(), APInt(32, 0, true));
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
					v = ConstantInt::get(getGlobalContext(), APInt(32, 0, true));

				std::vector<Value *> idxList;
				idxList.push_back(ConstantInt::get(getGlobalContext(), APInt(32, 0, true)));
				idxList.push_back(ConstantInt::get(getGlobalContext(), APInt(64, i, true)));

				Value *arrayItemPtr = Builder.CreateGEP(arrayPtr, idxList, "array_init_" + *name);
				Builder.CreateStore(v, arrayItemPtr);
			}
		}

		if (node->isConstant) {
			std::map<std::string, AllocaInst *> &ConstLocalVariables = *ConstLocalTableStack[StackPtr-1];
			ConstLocalVariables[*name] = arrayPtr;
		}
		else {
			std::map<std::string, AllocaInst *> &LocalVariables = *LocalTableStack[StackPtr-1];
			LocalVariables[*name] = arrayPtr;
		}
	}
	if (node->hasSize)
		pending.pop_back();		// pop sizeV
}


void CodegenVisitor::visitEmptyNode(EmptyNode *node)
{
}


void CodegenVisitor::visitBlockNode(BlockNode *node)
{
}


void CodegenVisitor::visitConstDeclNode(ConstDeclNode *node)
{
}


void CodegenVisitor::visitVarDeclNode(VarDeclNode *node)
{
}


void CodegenVisitor::visitAssignStmtNode(AssignStmtNode *node)
{
	bool isConst;
	node->exp->accept(*this);
	Value *expV = pending.back();
	pending.pop_back();
	if (expV == 0)
		return;



	// "a = b + 1;"
	if (node->lval->type == ID_AST) {
		IdNode *lval = dynamic_cast<IdNode *>(node->lval);
		Value *lvalV = lookUp(*(lval->name), isConst);
		Builder.CreateStore(expV, lvalV);
	}
	// "a[4] = b + 1;"
	else {
		ArrayItemNode *arrayItemNode = dynamic_cast<ArrayItemNode *>(node->lval);

		Value *arrayPtr = lookUp(*(arrayItemNode->name), isConst);

		arrayItemNode->index->accept(*this);
		Value *indexV = pending.back();
		pending.pop_back();
		if (indexV == 0)
			return;

		std::vector<Value *> idxList;
		idxList.push_back(ConstantInt::get(getGlobalContext(), APInt(32, 0, true)));
		Value *sextIndex = Builder.CreateSExt(indexV, Type::getInt64Ty(getGlobalContext()));
		idxList.push_back(sextIndex);

		Value *arrayItemPtr = Builder.CreateGEP(arrayPtr, idxList, "array_ptr");

		Builder.CreateStore(expV, arrayItemPtr);
	}
}


void CodegenVisitor::visitFunCallStmtNode(FunCallStmtNode *node)
{
	// Look up the name in the global module table.
	Function *calleeF = TheModule->getFunction(*(node->name));

	std::list<Node *> arguments;
	if (node->hasArgs)
		arguments = node->argv->nodes;

	std::vector<Value *> argsV = getValuesFromStack(arguments.size());

	Value *retV = Builder.CreateCall(calleeF, argsV);
	pending.insert(pending.end(), retV);
}


void CodegenVisitor::visitBlockStmtNode(BlockStmtNode *node)
{
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
	pending.insert(pending.end(), retV);
}


void CodegenVisitor::visitIfStmtNode(IfStmtNode *node)
{
	// enter new scope
	LocalTableStack[StackPtr] = new std::map<std::string, AllocaInst *>;
	ConstLocalTableStack[StackPtr] = new std::map<std::string, AllocaInst *>;
	StackPtr++;

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

	// exit current scope
	StackPtr--;
	delete LocalTableStack[StackPtr];
	delete ConstLocalTableStack[StackPtr];
}


void CodegenVisitor::visitWhileStmtNdoe(WhileStmtNode *node)
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


	// exit current scope
	StackPtr--;
	delete LocalTableStack[StackPtr];
	delete ConstLocalTableStack[StackPtr];
}


void CodegenVisitor::visitBreakStmtNode(BreakStmtNode *node)
{
}


void CodegenVisitor::visitContinueStmtNode(ContinueStmtNode *node)
{
}


void CodegenVisitor::visitFuncDeclNode(FuncDeclNode *node)
{
	std::string *name = node->name;
	std::list<Node *> argNames;
	if (node->hasArgs)
		argNames = node->argv->nodes;

	std::vector<Type *> Ints(argNames.size(), Type::getInt32Ty(getGlobalContext()));
	FunctionType *FT =
			FunctionType::get(Type::getVoidTy(getGlobalContext()), Ints, false);
	Function *F =
	      Function::Create(FT, Function::ExternalLinkage, name->c_str(), TheModule);

	// if F conflicted, there was already something named 'Name'.
	if (F->getName() != *name) {
		F->eraseFromParent();
		F = TheModule->getFunction(*name);
	}

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

	// create an alloca for each argument
	for (Function::arg_iterator aIt = F->arg_begin(); aIt != F->arg_end(); aIt++) {
		AllocaInst *alloca = Builder.CreateAlloca(Type::getInt32Ty(getGlobalContext()), 0, aIt->getName());
		Builder.CreateStore(aIt, alloca);
		LocalVariables[aIt->getName()] = alloca;
	}


	node->block->accept(*this);


    Builder.CreateRetVoid();

    // validate the generated code, checking for consistency
	verifyFunction(*F);

	// optimize the function
	//TheFPM->run(*F);

	// exit current scope
	StackPtr--;
	delete LocalTableStack[StackPtr];
	delete ConstLocalTableStack[StackPtr];

	Builder.ClearInsertionPoint();
}


void CodegenVisitor::visitCompUnitNode(CompUnitNode *node)
{
}


void CodegenVisitor::enterBlockNode(BlockNode *node)
{
}


void CodegenVisitor::enterIfStmtNode(IfStmtNode *node)
{
}


void CodegenVisitor::enterWhileStmtNode(WhileStmtNode *node)
{
}


void CodegenVisitor::enterFuncDefNode(FuncDefNode *node)
{
}


