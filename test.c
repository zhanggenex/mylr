#include <stdio.h>
#include <time.h>
#include "/home/hunter-zg/mylr/encoded-errno/include/uapi/asm-generic/errno-base.h"
#include "/home/hunter-zg/mylr/encoded-errno/include/uapi/asm-generic/errno.h"

int x = 1;

int b()
{
	if ( x > 2)
	{	
		int c = x;
	}
	else 
	{
		return ENOSYS;
	}
}

int main()
{
	time_t start, stop;
	start = time(NULL);
	b();
	stop = time(NULL);
	x = stop -start;
	int a = x;
	return 1;
}
