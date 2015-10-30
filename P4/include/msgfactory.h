#include <cstdio>
#include <string>
#include <map>
#include <list>
#include "util.h"

using namespace std;

enum MsgType {
	// warnings
	w_miss_int,
	
	// errors
	e_rparent,
	e_lparent,
	e_miss_op,
	e_miss_semicolon
};

// base class of compiling message
class Message {
public:
	friend class MsgFactory;

	Message(int type, int line, int column, const string fileName) 
		: type(type), line(line), column(column), fileName(fileName) {} 

	// print this message to stdout in proper format 
	virtual void show(){}

protected:
	int type; 		// type of message, defined in MsgType
	int line; 		// line number
	int column; 	// column number
	string fileName;
};

// error message, inherited from Message
class Error : public Message {
public:
	friend class MsgFactory;

	Error(int type, int line, int column, const string fileName) 
		: Message(type, line, column, fileName) {} 

	void show();
};

// warning message, inherited from Message
class Warning : public Message {
public:
	friend class MsgFactory;

	Warning(int type, int line, int column, const string fileName) 
		: Message(type, line, column, fileName) {} 

	void show();
};

// a factory class handling the messages emitted during compilation
class MsgFactory{
public:
	MsgFactory();

	void initial(const char *fileName);

	Error newError(int type, int line, int column);
	Warning newWarning(int type, int line, int column);

	void showMsg(Message *msg);
	void summary();

private:
	string fileName;
	list<Error> errors;
	list<Warning> warnings;
	FILE *source;
	long lineOffset[65536];
};


