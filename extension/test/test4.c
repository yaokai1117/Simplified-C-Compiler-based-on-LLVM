
extern void print(int c);
extern void print_space();
extern void print_newline();

int label = 233;

int func()
{
	label = 450;
	return 3;
}

void main()
{

	int a = 1, b = 4, c = 3;
	
	if ((a > b) && (a > c))
		print(1);
	else if ((b > a) && (b > c))
		print(2);
	else
		print(3);
	print_newline();

	int i  = 0, j = 0;
	while (i < 10 || j < 200) {
		i = i + 1;
		j = j + 1;
	}

	print(i);
	print_newline();

	if (!(i < 0))
		print(1);
	else
		print(0);

	print_newline();

	if ((1 < 0) && (func() > 0))
		print(2333);
	else
		print(4500);
	
	print_newline();

	print(label);

	print_newline();

}
