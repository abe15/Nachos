/*Test 2:To test the Release and DestroyLock systemcalls */



#include "syscall.h"

int main()
{
	

int b;

Print("In the userprogram \n", 0, 0, 0);

b=CreateLock("Lock A",6);
Acquire(b); 
Release(b);
DestroyLock(b);

}
