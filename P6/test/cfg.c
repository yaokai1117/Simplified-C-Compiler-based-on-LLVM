int main()
{
	int a, n, ans, i;
	if (a >= 0)
		for (i = 0; i < n; i++)
			ans = ans + i;
	else
		while (a < 0)
			a = a + n;
	return 0;
}
