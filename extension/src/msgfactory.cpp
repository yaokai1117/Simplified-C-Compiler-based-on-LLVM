#include <cstdio>
#include <string>
#include <map>
#include "util.h"
#include "msgfactory.h"

using namespace std;

map<int, string> createMsgTable()
{
	map<int, string> t;
	// warnings
	t[w_miss_int] = string("a type specifier 'int' is required for declarations");

	// errors
	t[e_rparent] = string("expected ')'");
	t[e_lparent] = string("expected '('");
	t[e_miss_op] = string("expected an operation");
	t[e_miss_semicolon] = string("expected ';'");
	t[e_const_decl_not_init] = string("a constant should be initialized during its declaration");

	t[e_global_init_not_constant] = string("global variables can only be initialized by constant value");
	t[e_undeclared_identifier] = string("use of undeclared identifier");
	t[e_redefinition_of_function] = string("redefinition fo function");
	t[e_unknown_function] = string("unknown function reference");
	t[e_redefinition_of_identifier] = string("redefinition of identifier");
	t[e_assign_to_constant] = string("assignment to a constant is illegal");
	t[e_argument_unmatch] = string("function argument unmatch");
	t[e_array_size_not_constant] = string("the size of array should be constant");
	return t;
}

// MsgTable map message type (int) to debug info (string)
map<int, string> MsgTable = createMsgTable();


// implementation of method in Error class
void Error::show()
{
	fprintf(stdout, "\033[31m""%s\n""\033[0m", MsgTable[type].c_str());
}


// implementation of method in Warning class
void Warning::show()
{
	fprintf(stdout, "\033[33m""%s\n""\033[0m", MsgTable[type].c_str());
}


// implementation of method in MsgFactory class
MsgFactory::MsgFactory()
{
}

MsgFactory::~MsgFactory()
{
	fclose(source);	
}

void MsgFactory::initial(const char *fileName)
{
	this->fileName = fileName;

	source = fopen(fileName, "r");
	if (source == NULL) {
		fprintf(stdout, "MsgFactory can not open source file %s\n", fileName);
		return;
	}

	// record the start location of every line
	int i = 0;
	char buffer[500];
	while (feof(source) == 0) {
		lineOffset[i++] = ftell(source);
		fgets(buffer, 500, source);
	}
	fseek(source, 0, SEEK_SET);
}

Error MsgFactory::newError(int type, int line, int column)
{
	Error e(type, line, column, fileName);
	errors.push_back(e);
	return e;
}

Warning MsgFactory::newWarning(int type, int line, int column)
{
	Warning w(type, line, column, fileName);
	warnings.push_back(w);
	return w;
}

void MsgFactory::showMsg(Message *msg)
{
	char buffer[500];
	int line = msg->line;
	int column = msg->column;

	fseek(source, lineOffset[line-1], SEEK_SET);
	fgets(buffer, 500, source);

	fprintf(stdout,"\033[0m" "%s: %d:%d: " "\033[0m", fileName.c_str(), msg->line, msg->column);
	msg->show();


	// change tab into four spaces
	char errorLine[500], positionLine[500];
	int i, j, cnt;
	for (i = 0, j = 0, cnt = 0; buffer[i] != '\0'; j++) {
		if (buffer[i] == '\t' && cnt < 3) {
			errorLine[j] = ' ';
			positionLine[j] = ' ';
			cnt++;
			continue;
		}
		if (cnt == 3) {
			cnt = 0;
			errorLine[j] = ' ';
			positionLine[j] = (i+1 == column) ? '^' : ' ';
			i++;
			continue;
		}
		errorLine[j] = buffer[i];
		positionLine[j] = ' ';
		if (i+1 == column)
			positionLine[j] = '^';
		i++;
	}
	errorLine[j] = '\0';
	positionLine[j] = '\0';

	fprintf(stdout,"\033[0m" "%s\n" "\033[0m", errorLine);
	fprintf(stdout,"\033[0m" "%s\n" "\033[0m", positionLine);
}

void MsgFactory::summary()
{
	for (list<Warning>::iterator it = warnings.begin(); it != warnings.end(); it++)
		showMsg(&*it);
	for (list<Error>::iterator it = errors.begin(); it != errors.end(); it++)
		showMsg(&*it);
	fprintf(stdout,"\033[0m" "compiling completed: totally %lu errors, %lu warnings\n" "\033[0m", errors.size(), warnings.size());
}

/*
int main()
{
	MsgFactory msgFactory;
	msgFactory.initial("test/test1.c");
	Error e1 = msgFactory.newError(e_miss_op, 2, 10);
	msgFactory.showMsg(&e1);
	return 0;
}
*/
