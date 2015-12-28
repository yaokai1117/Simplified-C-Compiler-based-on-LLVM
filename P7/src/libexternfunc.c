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

void print_char(int c) 
{
	printf("%c", c);
}
