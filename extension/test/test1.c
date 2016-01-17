
extern void print(int c);
extern void print_char(char c);
extern void print_float(float c);
extern void print_newline();

struct Sa{
	int id;
	float score;
	char gender;
};

void showStruct(struct Sa a)
{
	print(a.id);
	print_char(' ');
	print_float(a.score);
	print_char(' ');
	print_char(a.gender);
	print_char('\n');
}

void main()
{
	struct Sa sa;
	sa.id = 233;
	sa.score = 4.3;
	sa.gender = 'f';
	showStruct(sa);

	struct Sa sb = sa;
	showStruct(sb);

	print_newline();

	struct Sa *pa = &sa;
	pa->id = 450;
	showStruct(sa);
}

