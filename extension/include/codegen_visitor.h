#ifndef _CODEGEN_VISITOR_H_
#define _CODEGEN_VISITOR_H_

#include <string>
#include <map>
#include <vector>
#include "visitor.h"
#include "node.h"

namespace llvm {
class Value;
class AllocaInst;
class GlobalVariable;
class BasicBlock;
}

class CodegenVisitor : public Visitor {
public:
	CodegenVisitor(std::string output_filename);
	~CodegenVisitor();

	void dump();

	virtual void visitNodeList(NodeList *node);
	virtual void visitNumNode(NumNode *node);
	virtual void visitFNumNode(FNumNode *node);
	virtual void visitCharNode(CharNode *node);
	virtual void visitBinaryExpNode(BinaryExpNode *node);
	virtual void visitUnaryExpNode(UnaryExpNode *node);
	virtual void visitIdNode(IdNode *node);
	virtual void visitArrayItemNode(ArrayItemNode *node);
	virtual void visitStructItemNode(StructItemNode *node);
	virtual void visitFunCallNode(FunCallNode *node);
	virtual void visitIdVarDefNode(IdVarDefNode *node);
	virtual void visitArrayVarDefNode(ArrayVarDefNode *node);
	virtual void visitEmptyNode(EmptyNode *node);
	virtual void visitBlockNode(BlockNode *node);
	virtual void visitVarDeclNode(VarDeclNode *node);
	virtual void visitAssignStmtNode(AssignStmtNode *node);
	virtual void visitFunCallStmtNode(FunCallStmtNode *node);
	virtual void visitBlockStmtNode(BlockStmtNode *node);
	virtual void visitCondNode(CondNode *node);
	virtual void visitIfStmtNode(IfStmtNode *node);
	virtual void visitWhileStmtNdoe(WhileStmtNode *node);
	virtual void visitReturnStmtNdoe(ReturnStmtNode *node);
	virtual void visitBreakStmtNode(BreakStmtNode *node);
	virtual void visitContinueStmtNode(ContinueStmtNode *node);
	virtual void visitFuncDeclNode(FuncDeclNode *node);
	virtual void visitFuncDefNode(FuncDefNode *node);
	virtual void visitStructDefNode(StructDefNode *node);
	virtual void visitCompUnitNode(CompUnitNode *node);

	virtual void enterBlockNode(BlockNode *node);
	virtual void enterIfStmtNode(IfStmtNode *node);
	virtual void enterWhileStmtNode(WhileStmtNode *node);
	virtual void enterFuncDefNode(FuncDefNode *node);
	virtual void enterStructDefNode(StructDefNode *node);

private:
	std::map<std::string, llvm::AllocaInst *> *ConstLocalTableStack[32];
	std::map<std::string, llvm::AllocaInst *> *LocalTableStack[32];
	std::map<std::string, llvm::GlobalVariable *> GloblalVariables;
	int StackPtr;

	std::vector<llvm::Value *> pending;
	llvm::BasicBlock *funcEndBB;
	llvm::AllocaInst *returnValue;

	llvm::Value *lookUp(std::string nameStr, bool &isConst);
	std::vector<llvm::Value *> getValuesFromStack(int size);
};




#endif /* _CODEGEN_VISITOR_H_ */
