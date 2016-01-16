#include <cstdio>
#include <string>
#include "msgfactory.h"
#include "node.h"
#include "tok.h"
#include "util.h"
#include "global.h"
#include "dumpdot_visitor.h"
#include "codegen_visitor.h"
#include "check_visitor.h"

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

#include "llvm/Analysis/Passes.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/PassManager.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"

extern FILE *yyin;

extern int yylex();     // lexer.cc provides yylex()
extern int yyparse();   // parser.cc provides yyparse()
extern int yylex_destroy();
extern void clearAstNodes();

CompUnitNode *root; // AST's root, shared with yyparse()
list<Node*> astNodes;
bool errorFlag = false;

MsgFactory msgFactory;

llvm::Module *TheModule;
llvm::ExecutionEngine *TheExecutionEngine;
llvm::FunctionPassManager *TheFPM;

int main(int argc, char** argv)
{
    if (handle_opt(argc, argv) == false)
        return 0;
    yyin = infp;        // infp is initialized in handle_opt()
    yyparse();

    if (!errorFlag) {
    	CheckVisitor checkVisitor;
    	root->accept(checkVisitor);
    }

    if (dumpfp != NULL && !errorFlag) {
        DumpDotVisitor dumpVisitor(dumpfp);
        root->accept(dumpVisitor);
    }

    // codegen
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    std::string inFileStr = string(infile_name);
    std::string ll_file_name = inFileStr.
    								substr(inFileStr.find("/") + 1, inFileStr.rfind('.') - inFileStr.find("/") - 1)
									+ string(".ll");
    llvm::LLVMContext &Context = llvm::getGlobalContext();
    std::unique_ptr<llvm::Module> Owner = llvm::make_unique<llvm::Module>("Yao Kai's compiler !!!", Context);
    TheModule = Owner.get();

    std::string ErrStr;
    TheExecutionEngine =
          llvm::EngineBuilder(std::move(Owner))
              .setErrorStr(&ErrStr)
              .setMCJITMemoryManager(llvm::make_unique<llvm::SectionMemoryManager>())
              .create();
    if (!TheExecutionEngine) {
        fprintf(stdout, "Could not create ExecutionEngine: %s\n", ErrStr.c_str());
        exit(1);
    }

    llvm::FunctionPassManager OurFPM(TheModule);

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

    if (!errorFlag) {
//    if (false) {
    	CodegenVisitor codegenVisitor(ll_file_name);
    	root->accept(codegenVisitor);

		if (!errorFlag) {
			freopen(ll_file_name.c_str(), "w", stderr);
			codegenVisitor.dump();
		}
    }
    // end codegen

    // messages
    msgFactory.summary();
    printf("\n");

	yylex_destroy();
	clearAstNodes();

	fclose(infp);
	if (dumpfp != NULL)
		fclose(dumpfp);

    return 0;
}
