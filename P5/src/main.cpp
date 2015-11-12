#include <cstdio>
#include <string>
#include "msgfactory.h"
#include "node.h"
#include "tok.h"
// comment the next line to hide debug info
#define DEBUG
#include "util.h"
#include "global.h"

extern FILE *yyin;
extern int yylex();     // lexer.cc provides yylex()
extern int yyparse();   // parser.cc provides yyparse()
extern int yylex_destroy();
extern void clearAstNodes();

extern MsgFactory msgFactory;
extern CompUnitNode *root; // AST's root, shared with yyparse()
extern list<Node*> astNodes;
extern bool errorFlag;

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

	msgFactory.summary();
	yylex_destroy();
	clearAstNodes();

	fclose(infp);
	if (dumpfp != NULL)
		fclose(dumpfp);

    return 0;
}
