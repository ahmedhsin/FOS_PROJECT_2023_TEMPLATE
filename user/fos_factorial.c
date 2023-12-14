
#include <inc/lib.h>


int factorial(int n);

void
_main(void)
{
	int i1=0;
	char buff1[256];
	i1 = 15;

	int res = factorial(i1) ;

	atomic_cprintf("Factorial %d = %d\n",i1, res);
	return;
}


int factorial(int n)
{
	if (n <= 1)
		return 1 ;
	return n * factorial(n-1) ;
}

