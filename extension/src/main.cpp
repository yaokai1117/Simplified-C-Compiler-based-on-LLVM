#include <cstdio>
#include <string>
#include "msgfactory.h"
#include "node.h"
#include "tok.h"
#include "util.h"
#include "global.h"
#include "dumpdot_visitor.h"
#include "codegen_visitor.h"

extern FILE *yyin;

extern int yylex();     // lexer.cc provides yylex()
extern int yyparse();   // parser.cc provides yyparse()
extern int yylex_destroy();
extern void clearAstNodes();

CompUnitNode *root; // AST's root, shared with yyparse()
list<Node*> astNodes;
bool errorFlag = false;

MsgFactory msgFactory;


int main(int argc, char** argv)
{
    if (handle_opt(argc, argv) == false)
        return 0;
    yyin = infp;        // infp is initialized in handle_opt()
    yyparse();


    if (dumpfp != NULL && !errorFlag) {
        DumpDotVisitor dumpVisitor(dumpfp);
        root->accept(dumpVisitor);
    }

    // codegen

    std::string inFileStr = string(infile_name);
    std::string ll_file_name = inFileStr.
    								substr(inFileStr.find("/") + 1, inFileStr.rfind('.') - inFileStr.find("/") - 1)
									+ string(".ll");

    if (!errorFlag) {
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
