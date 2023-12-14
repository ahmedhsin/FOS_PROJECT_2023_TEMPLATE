
#include <inc/lib.h>


int fibonacci(int n);

void
_main(void)
{
	int i1=0;
	char buff1[256];
	atomic_readline("Please enter a number:", buff1);
	i1 = strtol(buff1, NULL, 10);
	for(int i = 0; i<i1;i++)
		sys_create_env("fact",(myEnv->page_WS_max_size), (myEnv->SecondListSize),(myEnv->percentage_of_WS_pages_to_be_removed));
	return;
}


int fibonacci(int n)
{
	if (n <= 1)
		return 1 ;
	return fibonacci(n-1) + fibonacci(n-2) ;
}

