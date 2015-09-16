/*
 * definition of the type of tokens and var_val
 *
 */
#include <string>

#define ID 		1
#define NUM 	2
#define CONST 	3
#define INTDEF 	4
#define IF 		5
#define ELSE 	6
#define WHILE 	7
#define ODD 	8
#define LBRACE 	9
#define RBRACE 	10
#define LBRACKET 	11
#define RBRACKET 	12
#define LPARENT 	13
#define RPARENT 	14
#define EQUAL 	15
#define COMMA 	16
#define SEMICOLON 	17
#define RELOP 	18
#define OP 		20
//#define BINOP 	19
//#define UNARYOP 	20
#define EQ 	21
#define NEQ 	22
#define GT 		23
#define GTE 	24
#define LT 		25
#define LTE		26
#define QUOT 	27
#define SLASH 	28

extern std::string var_val;
extern int num_val;
extern int op_val;

