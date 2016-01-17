
extern void print(int c);
extern void print_char(char c);
extern void print_float(float c);
extern void print_newline();

struct Sa{
	int id;
	float score;
	char male;
};

/*
struct Sa sa;

int add(int a, int b)
{
	return a + b;
}

int sub(int a, int b)
{
	return a - b;
}

void change(int *a)
{
	*a = 233;
}

int compute(int a, int b, int (*op)(int a, int b))
{
	return op(a, b);
}
*/

void showStruct(struct Sa a)
{
	print(a.id);
	print_char(' ');
	print_float(a.score);
	print_char(' ');
	print_char(a.male);
	print_char('\n');
}

void main()
{
	struct Sa sa;
	sa.id = 233;
	sa.score = 4.3;
	sa.male = 'f';
	showStruct(sa);
}

