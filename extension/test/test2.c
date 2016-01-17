
extern void print(int c);
extern void print_char(char c);
extern void print_float(float f);
extern void print_newline();

int main()
{
	int a = 1;
	print(a); 		// 1
	print_newline();
	int *pa = &a;
	*pa = 2;
	print(a); 		// 2
	print_newline();
	int **ppa = &pa;
	**ppa = 3;
	print(a); 		// 3
	print_newline();

	int array[] = {1, 2, 3, 4, 5};
	int (*pArray)[3] = &array;

	print((*pArray)[4]); 	// 5
	print_newline();

	(*pArray)[0] = 100; 		
	print(array[0]); 		// 100
	print_newline();
}

