// a simple quick-sort test

int a[10] = {7, 4, 2, 8, 5, 3, 1, 6, 9, 0}; // the array to be sorted

int pivot;

extern void print(int c);
extern void print_space();
extern void print_newline();

void swap(int i, int j) 
{
	int temp = a[i];
	a[i] = a[j];
	a[j] = temp;
}

void partition(int p, int r)
{
	int key = a[r];
	int i = p - 1;
	int j = p;
	while (j <= r-1) {
		if (a[j] <= key) {
			i = i + 1;
			swap(i, j);
		}
		j = j + 1;
	}

	swap(i+1, r);
	pivot = i + 1;
}

void quick_sort(int p, int r)
{
	if (p < r) {
		partition(p, r);
		quick_sort(p, pivot - 1);
		quick_sort(pivot + 1, r);
	}
}


void main()
{

	int i = 0;
	while (i < 10) {
		print(a[i]);
		print_space();
		i = i + 1;
	}
	print_newline();

	quick_sort(0, 9);

	i = 0;
	while (i < 10) {
		print(a[i]);
		print_space();
		i = i + 1;
	}
	print_newline();
	
}

