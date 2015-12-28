
extern void print(int c);
extern void print_space();
extern void print_newline();

const int size = 5;

int a[size + 3] = {2, 0, 1, 5, 1, 2, 2, 8};

void main()
{

	int i = 0;
	while (i < 8) {
		print(a[i]);
		i = i + 1;
	}

	print_newline();

	int b[size];
	b[0] = 0;
	b[1] = 1;
	b[2] = 2;
	b[3] = 3;
	b[4] = 4;

	i = 0;
	while (i < size) {
		print(b[i]);
		i = i + 1;
	}
	print_newline();
}

