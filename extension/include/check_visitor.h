#ifndef _CHECK_VISITOR_H_
#define _CHECK_VISITOR_H_

#include "visitor.h"
#include <map>


class CheckVisitor : public Visitor {
public:
	CheckVisitor();
	~CheckVisitor();

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
	std::vector<ValueTypeS> pending;

	std::map<std::string, ValueTypeS> *symTableStack[32];
	std::map<std::string, ValueTypeS> globalSymTabble;
	int stackPtr;

	bool isGlobal;
	ValueTypeS lookUpSym(std::string name);
	void handleArrayType(ValueTypeS *vType);
};



#endif /* _CHECK_VISITOR_H_ */
