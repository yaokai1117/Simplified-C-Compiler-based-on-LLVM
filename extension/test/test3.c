extern void print(int c);
extern void print_char(char c);
extern void print_float(float c);
extern void print_newline();

int add(int a, int b)
{
	return a + b;
}

int sub(int a, int b)
{
	return a - b;
}

int compute(int a, int b, int (*op)(int a, int b))
{
	return op(a, b);
}

int main()
{
	print(compute(4, 2, add)); // 4 + 2 = 6
	print_newline();
	print(compute(4, 2, sub)); // 4 - 2 = 2
	print_newline();
}
