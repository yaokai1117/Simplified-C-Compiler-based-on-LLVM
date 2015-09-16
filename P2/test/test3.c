
int a[10];

void sort()
{
	int i, j, key;
	i = 0;
	j = 0;
	key = 0;

	while (i < 10) {
		j = i - 1;
		key = a[i];
		while ((j > 0) * (a[j] > key)) {
			a[j+1] = a[j];
			j = j - 1;
		}		
		a[j] = key;
		i = i + 1;
	}

}

void main()
{
	int i;
	i = 0;

	while (i < 10) {
		a[i] = 9 - i;
		printf("%d", a[i]);
		i = i + 1;
	}
	printf("\n");

	sort();

	i = 0;
	while (i < 10) {
		printf("%d", i);
		i = i + 1;
	}
	printf("\n");
}
