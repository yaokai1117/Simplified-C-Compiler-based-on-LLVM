extern void print(int c);
extern void print_char(char c);
extern void print_float(float c);
extern void print_newline();

struct Student {
	int id;
	float score;
	char name;
};

int compareById(struct Student *a, struct Student *b)
{
	if (a->id < b->id)
		return 1;
	else
		return 0;
}

int compareByScore(struct Student *a, struct Student *b)
{
	if (a->score < b->score)
		return 1;
	else
		return 0;
}

int compareByName(struct Student *a, struct Student *b)
{
	if (a->name < b->name)
		return 1;
	else
		return 0;
}

void sort(struct Student *a[5], int (*compare)(struct Student *a, struct Student *b))
{
	int i = 1;
	while (i < 5) {
		struct Student *key = a[i];
		int j = i - 1;
		while (j >= 0 && compare(key, a[j]) > 0) {
			a[j+1] = a[j];
			j = j - 1;
		}
		a[j+1] = key;
		i = i + 1;
	}
}


void showStruct(struct Student a)
{
	print(a.id);
	print_char('\t');
	print_float(a.score);
	print_char('\t');
	print_char(a.name);
	print_char('\n');
}

void showStructArray(struct Student *a[5])
{
	int i = 0;
	while (i < 5) {
		showStruct(*a[i]);
		i = i + 1;
	}
}

int main()
{
	struct Student *stuArray[5];
	struct Student s0, s1, s2, s3, s4;
	stuArray[0] = &s0;
	stuArray[1] = &s1;
	stuArray[2] = &s2;
	stuArray[3] = &s3;
	stuArray[4] = &s4;

	// init
	int i = 0;
	while (i < 5) {
		stuArray[i]->id = i;
		stuArray[i]->score = 100 * i;
		stuArray[i]->name = 'e' - i;
		i = i + 1;
	}

	stuArray[3]->score = 175;
	stuArray[1]->score = 450;
	stuArray[0]->score = 333;


	sort(stuArray, compareById);
	showStructArray(stuArray);
	print_newline();

	sort(stuArray, compareByScore);
	showStructArray(stuArray);
	print_newline();

	sort(stuArray, compareByName);
	showStructArray(stuArray);
	print_newline();
}

