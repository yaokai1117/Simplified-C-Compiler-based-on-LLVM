#ifndef _DUMPDOT_VISITOR_H_
#define _DUMPDOT_VISITOR_H_

#include <cstdio>
#include <vector>
#include "visitor.h"
#include "node.h"

class DumpDOT;

class DumpDotVisitor : public Visitor {
public:
	DumpDotVisitor(FILE *file);
	~DumpDotVisitor();

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
	DumpDOT *dumper;
	std::vector<int> pending;

	void dumpList(int length, int nRoot, int pos);
};



#endif /* _DUMPDOT_VISITOR_H_ */
