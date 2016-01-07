#ifndef _VISITOR_H_
#define _VISITOR_H_

#include "node.h"

class Visitor {
public:
	Visitor(){orderChanged = false;}
	virtual ~Visitor(){};


	virtual void visitNodeList(NodeList *node) = 0;
	virtual void visitNumNode(NumNode *node) = 0;
	virtual void visitFNumNode(FNumNode *node) = 0;
	virtual void visitCharNode(CharNode *node) = 0;
	virtual void visitBinaryExpNode(BinaryExpNode *node) = 0;
	virtual void visitUnaryExpNode(UnaryExpNode *node) = 0;
	virtual void visitIdNode(IdNode *node) = 0;
	virtual void visitArrayItemNdoe(ArrayItemNode *node) = 0;
	virtual void visitFunCallNode(FunCallNode *node) = 0;
	virtual void visitIdVarDefNode(IdVarDefNode *node) = 0;
	virtual void visitArrayVarDefNode(ArrayVarDefNode *node) = 0;
	virtual void visitEmptyNode(EmptyNode *node) = 0;
	virtual void visitBlockNode(BlockNode *node) = 0;
	virtual void visitVarDeclNode(VarDeclNode *node) = 0;
	virtual void visitAssignStmtNode(AssignStmtNode *node) = 0;
	virtual void visitFunCallStmtNode(FunCallStmtNode *node) = 0;
	virtual void visitBlockStmtNode(BlockStmtNode *node) = 0;
	virtual void visitCondNode(CondNode *node) = 0;
	virtual void visitIfStmtNode(IfStmtNode *node) = 0;
	virtual void visitWhileStmtNdoe(WhileStmtNode *node) = 0;
	virtual void visitReturnStmtNdoe(ReturnStmtNode *node) = 0;
	virtual void visitBreakStmtNode(BreakStmtNode *node) = 0;
	virtual void visitContinueStmtNode(ContinueStmtNode *node) = 0;
	virtual void visitFuncDeclNode(FuncDeclNode *node) = 0;
	virtual void visitFuncDefNode(FuncDefNode *node) = 0;
	virtual void visitCompUnitNode(CompUnitNode *node) = 0;

	virtual void enterBlockNode(BlockNode *node) = 0;
	virtual void enterIfStmtNode(IfStmtNode *node) = 0;
	virtual void enterWhileStmtNode(WhileStmtNode *node) = 0;
	virtual void enterFuncDefNode(FuncDefNode *node) = 0;

	bool orderChanged;

private:
};



#endif /* _VISITOR_H_ */
