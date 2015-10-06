#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <map>
#include <string>
#include <vector>
#include <sstream>
#include "ast.h"
#include "dumpdot.h"

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
    for (int i=0; i<list.size(); i++) {
        std::string st = list[i];
        if (!first)
            fprintf(fp, "|");
        first = false;
        fprintf(fp, "<%d> %s", i, st.c_str());
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

//===----------------------------------------------------------------------===//
// Dump AST to DOT
//===----------------------------------------------------------------------===//

// The following functions convert AST to DOT using DumpDOT.
// Each dumpdot returns an integer, which is corresponding number in DOT file.
// 53+29*71 will become:
// digraph {
// node [shape = record];
//     0 [label = "<0> |<1> +|<2> "];
//     1 [label = "<0> 53"];
//     2 [label = "<0> |<1> *|<2> "];
//     3 [label = "<0> 29"];
//     4 [label = "<0> 71"];
//     0:0 -> 1;
//     0:2 -> 2;
//     2:0 -> 3;
//     2:2 -> 4;
// }

namespace ast {

int NumberExprAST::dumpdot(DumpDOT *dumper) {
    std::ostringstream strs;
    strs << Val;
    int nThis = dumper->newNode(1, strs.str().c_str());
    return nThis;
}

int VariableExprAST::dumpdot(DumpDOT *dumper) {
    int nThis = dumper->newNode(1, Name.c_str());
    return nThis;
}

int UnaryExprAST::dumpdot(DumpDOT *dumper) {
    char st[2] = " ";
    st[0] = Opcode;
    int nThis = dumper->newNode(2, st, " ");
    int nOperand = Operand->dumpdot(dumper);
    dumper->drawLine(nThis, 1, nOperand);
    return nThis;
}

int BinaryExprAST::dumpdot(DumpDOT *dumper) {
    char st[2] = " ";
    st[0] = Op;
    int nThis = dumper->newNode(3, " ", st, " ");
    int nLHS = LHS->dumpdot(dumper);
    int nRHS = RHS->dumpdot(dumper);
    dumper->drawLine(nThis, 0, nLHS);
    dumper->drawLine(nThis, 2, nRHS);
    return nThis;
}

int CallExprAST::dumpdot(DumpDOT *dumper) {
    std::vector<std::string> vec;
    vec.push_back(Callee);
    for (int i=0; i<Args.size(); i++)
         vec.push_back(" ");
    int nThis = dumper->newNode(vec);
    for (int i=0; i<Args.size(); i++) {
        ExprAST *arg = Args[i];
        int nArg = arg->dumpdot(dumper);
        dumper->drawLine(nThis, i+1, nArg); 
    }
    return nThis;
}

int IfExprAST::dumpdot(DumpDOT *dumper) {
    int nThis = dumper->newNode(3, "if-cond", "then", "else");
    int nCond = Cond->dumpdot(dumper);
    int nThen = Then->dumpdot(dumper);
    int nElse = Else->dumpdot(dumper);
    dumper->drawLine(nThis, 0, nCond);
    dumper->drawLine(nThis, 1, nThen);
    dumper->drawLine(nThis, 2, nElse);
    return nThis;
}

int WhileExprAST::dumpdot(DumpDOT *dumper) {
	int nThis = dumper->newNode(2, "while-cond", "body");
	int nCond = Cond->dumpdot(dumper);
	int nBody = Body->dumpdot(dumper);
	dumper->drawLine(nThis, 0, nCond);
	dumper->drawLine(nThis, 1, nBody);
	return nThis;
}

int ForExprAST::dumpdot(DumpDOT *dumper) {
    int nThis = dumper->newNode(5, VarName.c_str(), "start", "end", "step", "body");
    int nStart = Start->dumpdot(dumper);
    int nEnd = End->dumpdot(dumper);
    int nBody = Body->dumpdot(dumper);
    dumper->drawLine(nThis, 1, nStart);
    dumper->drawLine(nThis, 2, nEnd);
    dumper->drawLine(nThis, 4, nBody);
    if (Step) {
        int nStep = Step->dumpdot(dumper);
        dumper->drawLine(nThis, 3, nStep);
    }
    return nThis;
}

int VarExprAST::dumpdot(DumpDOT *dumper) {
    std::vector<std::string> vec;
    std::vector<int> arg;
    vec.push_back("var");
    for (std::pair<std::string, ExprAST*> p : VarNames) {
        vec.push_back(p.first);
        if (p.second == NULL)
            arg.push_back(-1);
        else
            arg.push_back(p.second->dumpdot(dumper));
    }
    vec.push_back("body");
    int nBody = Body->dumpdot(dumper);
    int nThis = dumper->newNode(vec);
    int i = 1;
    for (int n : arg) {
        if (n >= 0)
            dumper->drawLine(nThis, i, n);
        i++;
    }
    dumper->drawLine(nThis, i, nBody);
    return nThis;
}

int PrototypeAST::dumpdot(DumpDOT *dumper) {
    std::vector<std::string> vec;
    vec.push_back(Name);
    for (std::string st : Args)
        vec.push_back(st);
    int nThis = dumper->newNode(vec);
    return nThis;
}

int FunctionAST::dumpdot(DumpDOT *dumper) {
    int nThis  = dumper->newNode(2, "proto", "body");
    int nProto = Proto->dumpdot(dumper);
    int nBody  = Body->dumpdot(dumper);
    dumper->drawLine(nThis, 0, nProto);
    dumper->drawLine(nThis, 1, nBody);
    return nThis;
}

}

