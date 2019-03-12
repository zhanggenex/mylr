#include <stdio.h>

int b()
{
	int x = 1;
	if ( x > 2)
	{
		printf("x");
		return 3;
	}
	else return 2;
}

void a()
{
	for (int i = 1; i <= 5; i++ )
	{
		printf("%d\n", b());
	}
}

int main()
{
	a();
	return 0;
}
