
extern void print(int c);
extern void println(int c);
extern void print_space();
extern void print_newline();

void add(int a, int b)
{
	println(a + b);
}

void sub(int a, int b)
{
	println(a - b);
}

void multi(int a, int b)
{
	println(a * b);
}

void div(int a, int b)
{
	println(a / b);
}

void main()
{
	int a = 6, b = 2;
	print(a);
	print_space();
	println(b);

	add(a, b);
	sub(a, b);
	multi(a, b);
	div(a, b);
}
