
extern void print(int c);
extern void print_space();
extern void print_newline();

void main()
{
	int a = 1;
	int b = 2;

	if (a < b)
		print(a);
	else
		print(b);
	print_newline();

	int i = 0;
	while (i < 8) {
		print(i);
		print_space();
		i = i + 1;
	}
	print_newline();
}

