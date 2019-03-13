#include <stdio.h>
#include <errno.h>

int b()
{
	FILE *fp = NULL;
	fp = fopen("./test.c", "r");
	if ( fp == NULL)
	{	
		/*if( x < 10)
		{
			c(2);
		
			if ( x < 100)	
				d(3);
			return 1;
		}
		if(x < 2)
			printf("1\n");
		return 1;*/
	}
	else 
	{
		return 20;
	}
}

int main()
{
	b();
	return 1;
}
