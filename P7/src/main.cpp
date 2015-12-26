#include <cstdio>
#include <string>
#include "msgfactory.h"
#include "node.h"
#include "tok.h"
#include "util.h"
#include "global.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

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

int main(int argc, char** argv)
{
    if (handle_opt(argc, argv) == false)
        return 0;
    yyin = infp;        // infp is initialized in handle_opt()
    yyparse();


    if (dumpfp != NULL && !errorFlag) {
        DumpDOT *dumper = new DumpDOT(dumpfp);
        root->dumpdot(dumper);
        delete dumper;
    }

    // debug
    llvm::LLVMContext &Context = llvm::getGlobalContext();
    TheModule = new llvm::Module("my cool jit", Context);

    if (!errorFlag)
    	root->codegen();
    if (!errorFlag)
    	TheModule->dump();
    // end debug

    msgFactory.summary();

	yylex_destroy();
	clearAstNodes();

	fclose(infp);
	if (dumpfp != NULL)
		fclose(dumpfp);

    return 0;
}
