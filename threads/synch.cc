// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!


//**********Lock Part************


Lock::Lock(char* debugName) 		//Lock's Constructor
{	
	/**********CREATED STARTS HERE*************/

	name = debugName;		//Lock given a name for debugging purpose
	waitqueue = new List;		//waitqueue is the wait queue for this NACHOS Lock

	status = Free;			//Lock's status set to Free

	ownerThread=NULL;		//initially Lock is Free & not owned by anyone
}					//so ownerThread is initialized to NULL


Lock::~Lock()				//Lock's Destructor
{
	delete waitqueue;		//deallocates the waitqueue used throughout
}					//the lock's life



void Lock::Acquire()			//Method run to assign Lock to a Thread
{
	IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
		if ( isHeldByCurrentThread() )				//Checks if the current Thread owns the Lock
    		{
			;
		}
    	else if ( status == Free )				//checks if the Lock is Available
	{
		status = Busy;					//changes Lock Status to Busy &		
								//stores prev status in status 
		ownerThread=currentThread;			//Allocates Lock to the current Thread
	}
	
	else
	{							//Lock is already Busy
		waitqueue->Append((void *)currentThread);	//Current Thread is added to the Lock's wait queue
		currentThread->Sleep();				//Thread goes to sleep

	}

  	(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}


void Lock::Release() 			//Method run by a thread to release lock
{
	IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts

  	if (!isHeldByCurrentThread())				//The Current Thread doesn't own Lock
	{							//hence, it can't release the Lock
		
		DEBUG('t', "The Current Thread does not own the lock !");	//informative message for debugging
										//Message for the user displayed
	//	printf("Current Thread doesn't own the Lock !! Can't Release Lock\n");
	}


	else if ( ! (waitqueue->IsEmpty() ))			//a thread's waiting in the Lock's wait queue
	{
		
		Thread *thread;					//thread will point to a thread from the waiting queue
   
		thread = (Thread *)waitqueue->Remove();		//thread removed from the wait queue
		scheduler->ReadyToRun(thread);			//thread put in the ready queue

		ownerThread=thread;				//the thread now owns the lock
	
	}

	else
	{							//no thread's present in the lock's wait queue
		status = Free;
		
		ownerThread = NULL;				//Lock is not owned by any thread right now
	}
	

	(void) interrupt->SetLevel(oldLevel);			// re-enable interrupts
}



bool Lock::isHeldByCurrentThread()				//routine to check if the current thread owns lock
{
	if ( ownerThread == currentThread)
	{
		DEBUG('t', "The Current Thread owns the Lock");	//informative message for debugging
		
		return 1;					//function returns 1 if lock is owned
	}							//by current thread else 0
	
	return 0;
}





//**********MonitorPart************





Condition::Condition(char* debugName) 	//Condition Class's Constructor
{	
	name = debugName;		//Condition Variable's given a name for debugging purpose
	CVwaitqueue = new List;		//CVwaitqueue is the wait queue for this Condition Variable

	waitLock=NULL;			//Set to Null initially &  when no one is waiting, i.e. in the queue
					//else set to the Lock passed as argument in Wait, Signal or Broadcast
}


Condition::~Condition() 		//Condition Class's Destructor
{ 
	delete CVwaitqueue;		//deallocates the CVwaitqueue used throughout
					//the Condition Variable's life
}



void Condition::Wait(Lock* conditionLock) //Wait operation on the Condition Variable
{					//thread's suspended & put on CVwaitqueue		
	IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts

	if ( conditionLock == NULL )		//Invalid Lock passed as parameter
	{
		DEBUG('t', "Invalid Lock !!");	//informative message for debugging					
		printf("Invalid Lock !! \n");	//Message for the user displayed

		(void) interrupt->SetLevel(oldLevel);			// re-enable interrupts
		return;
	}
		
	
	if ( waitLock == NULL)			//First thread calling wait or queue's empty
		waitLock=conditionLock;		//The Lock is saved into the waitLock

	if ( waitLock != conditionLock )	//Input Lock doesn't match the saved Lock
	{

		DEBUG('t', "Input Lock doesn't match the saved Lock !!");	//informative message for debugging
		printf("wInput Lock doesn't match the saved Lock !! \n");	//Message for the user displayed
	
		(void) interrupt->SetLevel(oldLevel);			// re-enable interrupts
		return;
		
	}
						//By now, all conditions ahve been checked to allow
						//a thread to wait

	CVwaitqueue->Append((void *)currentThread);	//Current Thread is added to the Condition Variable's wait queue

	conditionLock->Release();			//Lock released by thread as it can not go to sleep while	
							//it still owns the Lock. This helps avoid Deadlock
	currentThread->Sleep();				//Thread goes to sleep

	conditionLock->Acquire();			//Lock acquired by thread as it owned the Lock before the wait 
							//operation was performed and is expected to still own it
							//at the time of its return


	(void) interrupt->SetLevel(oldLevel);		// re-enable interrupts
}
	

void Condition::Signal(Lock* conditionLock)	//Signal operation on the Condition Variable
{ 						//thread's woken up	
	IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts

	if ( CVwaitqueue->IsEmpty() )		//There's no one waiting in the Queue
	{
		DEBUG('t', "No one is waiting in the Queue !!");//informative message for debugging			
//		printf("No one is waiting in the Queue !! \n");	//Message for the user displayed

		(void) interrupt->SetLevel(oldLevel);			// re-enable interrupts
		return;
	}
		
	if ( waitLock != conditionLock )	//Input match doesn't match the saved Lock
	{
		DEBUG('t', "Input match doesn't match the saved Lock !!");//informative message for debugging			
		printf("sInput match doesn't match the saved Lock !! \n");	//Message for the user displayed

		(void) interrupt->SetLevel(oldLevel);			// re-enable interrupts
		return;
	}
			
	Thread *thread;				//thread will point to a thread from the waiting queue
   
	thread = (Thread *)CVwaitqueue->Remove();	//thread removed from the wait queue
	scheduler->ReadyToRun(thread);			//thread put in the ready queue
	
	
	if ( CVwaitqueue->IsEmpty() )			//There's no one waiting in the Queue
		waitLock=NULL;


	(void) interrupt->SetLevel(oldLevel);		// re-enable interrupts

}



void Condition::Broadcast(Lock* conditionLock)	//Broadcast operation on the Condition Variable
{						//signals all waiting threads
	while ( ! ( CVwaitqueue->IsEmpty() ))
		Signal(conditionLock);
}


















