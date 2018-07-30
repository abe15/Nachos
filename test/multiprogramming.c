#include "syscall.h"

void abc()
{
	Print("\nMultiprogramming is fun\n", 0, 0, 0);
	Exit(0);
}

int main()
{
	int i;
	Acquire(0);

	for (i=0; i<5; i++)
		Fork(abc);

	Wait(0, 0);
}


