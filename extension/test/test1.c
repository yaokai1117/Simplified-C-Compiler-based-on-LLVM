
extern void print(int c);
extern void print_char(char c);
extern void print_float(float c);
extern void print_newline();

struct Sa{
	int id;
	float score;
	char male;
};

struct Sa sa;

void main()
{
	struct Sa testS;
	float a;
	testS.id = 233;
	testS.score = 3.14;
	a = 2.34;
	print(testS.id);
	print_float(testS.score);
	print_char(' ');
	print_newline();
}

