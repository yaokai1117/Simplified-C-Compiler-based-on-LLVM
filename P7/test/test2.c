int a[10];

// sort function
void sort()
{
	int i, j, key;
	i = 1;
	while (i < 10) {
		key = a[i];
		j = i - 1;
		flag = 1;
		while (j >= 0) {
			if (a[j] <= key)
				flag = 0;
			if (flag > 0) {
				a[j+1] = a[j];
			}
			j = j - 1;
		}
		a[j+1] = key;
		i = i + 1;
	}
}

void main()
{
	int i;
	i = 0;

	while (i < 10) {
		a[i] = 9 - i;
		i = i + 1;
	}

	sort();

	i = 0;
	while (i < 10) {
		Output = a[i];
		print();
		i = i + 1;
	}
}
