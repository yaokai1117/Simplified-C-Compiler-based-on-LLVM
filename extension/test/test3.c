
extern void print(int c);
extern void println(int c);
extern void print_space();
extern void print_newline();

int add(int a, int b, int c)
{
	if (c == 0)
		return a+b;
	else if (c == 1)
		return a-b;
	else
		return a*b;
}

int max(int a, int b)
{
	if (a > b)
		return a;
	else
		return b;
}


void main()
{
	int a = 6, b = 2;
	print(a);
	print_space();
	println(b);

	println(add(a, b, 0));
	println(add(a, b, 1));
	println(add(a, b, 2));

	println(add(a, b, 0) + add(a, b, 1) + add(a, b, 3) -20);

	println(max(a, b));
}
