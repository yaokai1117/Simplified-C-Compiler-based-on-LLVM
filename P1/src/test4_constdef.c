
const int a_out = 2;
const int b[a_out] = {1, 2};

void main()
{
	const int a[] = {0, 1, 2};
	const int b = 2;
	const int c[b + 1] = {3, 4, 5};
}
