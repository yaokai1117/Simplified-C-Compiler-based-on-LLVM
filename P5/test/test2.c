
int a[10];

// sort function
void sort()
{
	int i, j, /*some
				comment blablabla */key;
	int flag;
	i = 0;
	j = 0;
	key = 0;
	flag = 1;

	/* today is a good day
	 * don't you think so?*/
	while (i < 10) {
		j = i - 1;
		key = a[i];
		while (j > 0) {
			if (a[j] <= key/*/jl*/)
				flag = 0;
			if (flag == 1) {
				a[j+1] = a[j];
				j = j - 1;
			}
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
		i = i + 1;
	}

	sort();

	i = 0;
	while (i < 10) {
		i = i + 1;
	}
}
