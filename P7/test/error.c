/*
 * This program has 4 errors and 1 warning
 **/
void main()
{
	int e[] = {3, 4};

	a = (b + c; 

	c = (a b + /* a long block comment,
				location of the error after the comment is correct */ d;

	if (a == 1) 
		if (b == 1)
			c = 1;
		else {
		b = 4;
		}

	const a = 3, b[20] = {3, 4};

	while (1 == 3) {
		b = 4 a;
	}

	hello();
}
