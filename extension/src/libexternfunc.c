#include <stdio.h>

// Declare a global variable

void println(int output) 
{
    printf("%d\n", output);
}

void print(int output) 
{
	printf("%d", output);
}

void print_space()
{
	printf("%c", ' ');
}

void print_newline()
{
	printf("\n");
}

void print_char(char c) 
{
	printf("%c", c);
}

void print_float(float f)
{
	printf("%.2f", f);
}
