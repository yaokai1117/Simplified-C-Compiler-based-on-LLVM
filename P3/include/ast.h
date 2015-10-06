#ifndef _AST_H_
#define _AST_H_

#include "llvm/IR/IRBuilder.h"
#include <cctype>
#include <cstdio>
#include <map>
#include <string>
#include <vector>
#include "dumpdot.h"

using namespace llvm;

//===----------------------------------------------------------------------===//
// Abstract Syntax Tree (aka Parse Tree)
//===----------------------------------------------------------------------===//
namespace ast {
/// ExprAST - Base class for all expression nodes.
class ExprAST {
public:
    virtual ~ExprAST() {}
    virtual Value *Codegen() = 0;
    virtual int dumpdot(DumpDOT *dumper) = 0;
};

/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
    double Val;

public:
    NumberExprAST(double val) : Val(val) {}
    virtual Value *Codegen();
    int dumpdot(DumpDOT *dumper);
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
    std::string Name;

public:
    VariableExprAST(const std::string &name) : Name(name) {}
    const std::string &getName() const { return Name; }
    virtual Value *Codegen();
    int dumpdot(DumpDOT *dumper);
};

/// UnaryExprAST - Expression class for a unary operator.
class UnaryExprAST : public ExprAST {
    char Opcode;
    ExprAST *Operand;

public:
    UnaryExprAST(char opcode, ExprAST *operand)
        : Opcode(opcode), Operand(operand) {}
    virtual Value *Codegen();
    int dumpdot(DumpDOT *dumper);
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
    char Op;
    ExprAST *LHS, *RHS;

public:
    BinaryExprAST(char op, ExprAST *lhs, ExprAST *rhs)
        : Op(op), LHS(lhs), RHS(rhs) {}
    virtual Value *Codegen();
    int dumpdot(DumpDOT *dumper);
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
    std::string Callee;
    std::vector<ExprAST *> Args;

public:
    CallExprAST(const std::string &callee, std::vector<ExprAST *> &args)
        : Callee(callee), Args(args) {}
    virtual Value *Codegen();
    int dumpdot(DumpDOT *dumper);
};

/// IfExprAST - Expression class for if/then/else.
class IfExprAST : public ExprAST {
    ExprAST *Cond, *Then, *Else;

public:
    IfExprAST(ExprAST *cond, ExprAST *then, ExprAST *_else)
        : Cond(cond), Then(then), Else(_else) {}
    virtual Value *Codegen();
    int dumpdot(DumpDOT *dumper);
};

/// WhileExprAST - Expression class for while Expr Expr
class WhileExprAST : public ExprAST {
	ExprAST *Cond, *Body;

public:
	WhileExprAST(ExprAST *cond, ExprAST *body)
		: Cond(cond), Body(body) {}
	virtual Value *Codegen();
	int dumpdot(DumpDOT *dumper);
};

/// ForExprAST - Expression class for for/in.
class ForExprAST : public ExprAST {
    std::string VarName;
    ExprAST *Start, *End, *Step, *Body;

public:
    ForExprAST(const std::string &varname, ExprAST *start, ExprAST *end,
            ExprAST *step, ExprAST *body)
        : VarName(varname), Start(start), End(end), Step(step), Body(body) {}
    virtual Value *Codegen();
    int dumpdot(DumpDOT *dumper);
};

/// VarExprAST - Expression class for var/in
class VarExprAST : public ExprAST {
    std::vector<std::pair<std::string, ExprAST *> > VarNames;
    ExprAST *Body;

public:
    VarExprAST(const std::vector<std::pair<std::string, ExprAST *> > &varnames,
            ExprAST *body)
        : VarNames(varnames), Body(body) {}

    virtual Value *Codegen();
    int dumpdot(DumpDOT *dumper);
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its argument names as well as if it is an operator.
class PrototypeAST {
    std::string Name;
    std::vector<std::string> Args;
    bool isOperator;
    unsigned Precedence; // Precedence if a binary op.
public:
    PrototypeAST(const std::string &name, const std::vector<std::string> &args,
            bool isoperator = false, unsigned prec = 0)
        : Name(name), Args(args), isOperator(isoperator), Precedence(prec) {}

    bool isUnaryOp() const { return isOperator && Args.size() == 1; }
    bool isBinaryOp() const { return isOperator && Args.size() == 2; }

    char getOperatorName() const {
        assert(isUnaryOp() || isBinaryOp());
        return Name[Name.size() - 1];
    }

    unsigned getBinaryPrecedence() const { return Precedence; }

    Function *Codegen();

    void CreateArgumentAllocas(Function *F);
    int dumpdot(DumpDOT *dumper);
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST {
    PrototypeAST *Proto;
    ExprAST *Body;

public:
    FunctionAST(PrototypeAST *proto, ExprAST *body) : Proto(proto), Body(body) {}

    Function *Codegen();
    int dumpdot(DumpDOT *dumper);
};
} // end ast namespace
#endif
