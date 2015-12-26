#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <string>
#include <vector>
#include <sstream>
#include "dumpdot.h"
#include "node.h"

//===----------------------------------------------------------------------===//
// Utilities in Dump DOT
//===----------------------------------------------------------------------===//

// There are two ways to create a dot node:
// 1. newNode(num, str1, str2, ...)
//    num corresponds to the number of strings
//    Each string will appear in the generated image as a port
//    All strings are char*
// 2. newNode(vec)
//    All elements of the vector are std::string
// newNode returns an integer, which is the number of the corresponding
// node in DOT file.

int DumpDOT::newNode(int num, ...) {
    va_list list;
    va_start(list, num);
    fprintf(fp, "    %d [label = \"", count);
    bool first = true;
    for (int i=0; i<num; i++) {
        char* st = va_arg(list, char*);
        if (!first)
            fprintf(fp, "|");
        first = false;
        if (st[0]=='<')
            fprintf(fp, "<%d> \\<", i);
        else
            fprintf(fp, "<%d> %s", i, st);
    }
    va_end(list);
    fprintf(fp, "\"];\n");
    return count++;
}

int DumpDOT::newNode(std::vector<std::string> list) {
    fprintf(fp, "    %d [label = \"", count);
    bool first = true;
    for (vector<std::string>::size_type i=0; i<list.size(); i++) {
        std::string st = list[i];
        if (!first)
            fprintf(fp, "|");
        first = false;
        fprintf(fp, "<%lu> %s", i, st.c_str());
    }
    fprintf(fp, "\"];\n");
    return count++;
}

// Draw a line from nSrc node's pSrc port to nDst node.
// If you want it start from the whole node, let pSrc = -1

void DumpDOT::drawLine(int nSrc, int pSrc, int nDst) {
    fprintf(fp, "    %d", nSrc);
    if (pSrc>=0)
        fprintf(fp, ":%d", pSrc);
    fprintf(fp, " -> %d;\n", nDst);
}


// Implementation of dumpdot methods in Node class

void NodeList::dumpdot(DumpDOT *dumper, int nRoot, int pos)
{
	for (std::list<Node*>::iterator it = nodes.begin(); it != nodes.end(); it++) {
		int nChild = (*it)->dumpdot(dumper);
		dumper->drawLine(nRoot, pos, nChild);
	}
}

int NodeList::dumpdot(DumpDOT *dumper)
{
	return 0;
}


int NumNode::dumpdot(DumpDOT *dumper)
{
	std::stringstream ss;
	ss << val;
	int nThis = dumper->newNode(1, ss.str().c_str());
	return nThis;
}


int BinaryExpNode::dumpdot(DumpDOT *dumper)
{
	char st[2] = " ";
	st[0] = op;
	int nThis = dumper->newNode(3, " ", st, " ");
	int nlhs = lhs->dumpdot(dumper);
	int nrhs = rhs->dumpdot(dumper);
	dumper->drawLine(nThis, 0, nlhs);
	dumper->drawLine(nThis, 2, nrhs);
	return nThis;
}


int UnaryExpNode::dumpdot(DumpDOT *dumper)
{
	char st[2] = " ";
	st[0] = op;
	int nThis = dumper->newNode(2, st, " ");
	int nOperand = operand->dumpdot(dumper);
	dumper->drawLine(nThis, 1, nOperand);
	return nThis;
}



int IdNode::dumpdot(DumpDOT *dumper)
{
	int nThis = dumper->newNode(1, name->c_str());
	return nThis;
}



int ArrayItemNode::dumpdot(DumpDOT *dumper)
{
	int nThis = dumper->newNode(4, name->c_str(), "\\[", " ", "\\]");
	int nIndex = index->dumpdot(dumper);
	dumper->drawLine(nThis, 2, nIndex);
	return nThis;
}



int IdConstDefNode::dumpdot(DumpDOT *dumper)
{
	int nThis = dumper->newNode(3, name->c_str(), "=", " ");
	int nValue = value->dumpdot(dumper);
	dumper->drawLine(nThis, 2, nValue);
	return nThis;
}


int ArrayConstDefNode::dumpdot(DumpDOT *dumper)
{
	int nThis = dumper->newNode(8, name->c_str(), "\\[", " ", "\\]", "=", "\\{", " ", "\\}");
	if (hasSize) {
		int nSize = size->dumpdot(dumper);
		dumper->drawLine(nThis, 2, nSize);
	}
	values->dumpdot(dumper, nThis, 6);
	return nThis;
}


int IdVarDefNode::dumpdot(DumpDOT *dumper)
{
	int nThis = 0;
	if (isAssigned) {
		nThis = dumper->newNode(3, name->c_str(), "=", " ");
		int nValue = value->dumpdot(dumper);
		dumper->drawLine(nThis, 2, nValue);
	}
	else {
		nThis = dumper->newNode(1, name->c_str());
	}
	return nThis;
}


int ArrayVarDefNode::dumpdot(DumpDOT *dumper)
{
	int nThis = 0;
	if (isAssigned) {
		nThis = dumper->newNode(8, name->c_str(), "\\[", " ", "\\]", "=", "\\{", " ", "\\}");
		if (hasSize) {
			int nSize = size->dumpdot(dumper);
			dumper->drawLine(nThis, 2, nSize);
		}
		values->dumpdot(dumper, nThis, 6);
	}
	else {
		nThis = dumper->newNode(4, name->c_str(), "\\[", " ", "\\]");
		int nSize = size->dumpdot(dumper);
		dumper->drawLine(nThis, 2, nSize);
	}
	return nThis;
}


int BlockNode::dumpdot(DumpDOT *dumper)
{
	int nThis = dumper->newNode(3, "\\{", " ", "\\}");
	blockItems->dumpdot(dumper, nThis, 1);
	return nThis;
}


int AssignStmtNode::dumpdot(DumpDOT *dumper)
{
	int nThis = dumper->newNode(3, " ", "=", " ");
	int nLVal = lval->dumpdot(dumper);
	int nExp = exp->dumpdot(dumper);
	dumper->drawLine(nThis, 0, nLVal);
	dumper->drawLine(nThis, 2, nExp);
	return nThis;
}


int FunCallStmtNode::dumpdot(DumpDOT *dumper)
{
	int nThis = dumper->newNode(2, name->c_str(), "\\( \\)");
	return nThis;
}


int BlockStmtNode::dumpdot(DumpDOT *dumper)
{
	return block->dumpdot(dumper);
}


int CondNode::dumpdot(DumpDOT *dumper)
{
	char op_str[4] = "   ";	
	switch (op) {
		case LT_OP:
			op_str[0] = '\\';
			op_str[1] = '<';
			break;
		case LTE_OP:
			op_str[0] = '\\';
			op_str[1] = '<';
			op_str[2] = '=';
			break;
		case GT_OP:
			op_str[0] = '\\';
			op_str[1] = '>';
			break;
		case GTE_OP:
			op_str[0] = '\\';
			op_str[1] = '>';
			op_str[2] = '=';
			break;
		case EQ_OP:
			op_str[1] = '=';
			op_str[2] = '=';
			break;
		case NEQ_OP:
			op_str[1] = '!';
			op_str[2] = '=';
			break;
	}
	int nThis = dumper->newNode(3, " ", op_str, " ");
	int nLhs = lhs->dumpdot(dumper);
	int nRhs = rhs->dumpdot(dumper);
	dumper->drawLine(nThis, 0, nLhs);
	dumper->drawLine(nThis, 2, nRhs);
	return nThis;
}


int IfStmtNode::dumpdot(DumpDOT *dumper)
{
	int nThis = 0;
	if (hasElse) {
		nThis = dumper->newNode(6, "if", " ", "then", " ", "else", " ");
		int nCond = cond->dumpdot(dumper);
		int nThen = then_stmt->dumpdot(dumper);
		int nElse = else_stmt->dumpdot(dumper);
		dumper->drawLine(nThis, 1, nCond);
		dumper->drawLine(nThis, 3, nThen);
		dumper->drawLine(nThis, 5, nElse);
	}
	else {
		nThis = dumper->newNode(4, "if", " ", "then", " ");
		int nCond = cond->dumpdot(dumper);
		int nThen = then_stmt->dumpdot(dumper);
		dumper->drawLine(nThis, 1, nCond);
		dumper->drawLine(nThis, 3, nThen);
	}
	return nThis;
}


int WhileStmtNode::dumpdot(DumpDOT *dumper)
{
	int nThis = dumper->newNode(3, "while", " ", " ");
	int nCond = cond->dumpdot(dumper);
	int nDo = do_stmt->dumpdot(dumper);
	dumper->drawLine(nThis, 1, nCond);
	dumper->drawLine(nThis, 2, nDo);
	return nThis;
}


int FuncDefNode::dumpdot(DumpDOT *dumper)
{
	int nThis = dumper->newNode(4, "void", name->c_str(), "\\( \\)", " ");
	int nBlock = block->dumpdot(dumper);
	dumper->drawLine(nThis, 3, nBlock);
	return nThis;
}


int ConstDeclNode::dumpdot(DumpDOT *dumper)
{
	int nThis = dumper->newNode(3, "const", "int", " ");
	defList->dumpdot(dumper, nThis, 2);
	return nThis;
}


int VarDeclNode::dumpdot(DumpDOT *dumper)
{
	int nThis = dumper->newNode(2, "int", " ");
	defList->dumpdot(dumper, nThis, 1);
	return nThis;
}


int CompUnitNode::dumpdot(DumpDOT *dumper)
{
	int nThis = dumper->newNode(1, "CompUnit");
	for (std::list<Node*>::iterator it = nodes.begin(); it != nodes.end(); it++) {
		int nUnit = (*it)->dumpdot(dumper);
		dumper->drawLine(nThis, 0, nUnit);
	}
	return nThis;
}


