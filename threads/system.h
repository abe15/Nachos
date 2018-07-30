// system.h 
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"
#include "synch.h"

#ifdef NETWORK
#define no_Per_Thread 5
#endif

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock


#ifdef NETWORK
extern int mailboxNoGenerator;			// Project 4 - Used to generate the mailbox IDs, which is unique for each 
						// thread on a nachos instance
extern int sequenceNoGenerator;			// Project 4 - Used to generate the sequence NOs, which is unique for each 
						// message on a nachos instance

extern Lock *mailboxNoGeneratorLock;
extern Lock *sequenceNoGeneratorLock;

struct serverDB					// Project 4 - Database at the Group Registration Server
{
	int machineID;
	int mailboxNO;
};


#endif


#ifdef USER_PROGRAM
#include "machine.h"
#define MAX_THREADS_PER_PROCESS 200		// Project 4 - Changed from 50 to 200
#define MAX_PROCESSES 10			// Project 4 - Changed from 5 to 10

#define MAX_LISTS 5


// *****************CREATED STARTS HERE****************
extern BitMap *pageBitMap;			//Page BitMap to find a free Page
extern Lock *pageTableLock;			//For the PageTable
extern Lock *memLock;				//Lock for the main memory


extern List* osLists[];
extern Lock *listLock;
 

extern int processUniqueID;
extern Lock *forkInitializationLock;
extern Lock *pInfoLock;

struct processInformationTable
{
	int noProcesses;
	int noThreads[MAX_PROCESSES];
};

extern processInformationTable pInfoTable;

//LOCK PART
extern int nextLockIndex;			//Increments each time a lock is created. Sort of like a Lock ID to 
						//ensure a unique integer number is available for each lock
extern Lock *kernelLockTableLock;		//osLocks[] is the kernel Table for Locks. This lock ensures mutual exclusion 
						//to this array
extern int MAX_LOCKS;				//Maximum number of Locks available

struct kernelLock			//Structure for the Kernel Lock Table. Holds details for each Lock
{
	Lock *lock;
	AddrSpace *as;
				
	int usageCounter;			//Will be initialized to 0 in Create Lock system call
	bool toBeDestroyed;			

};

extern kernelLock osLocks[];		 	//Array of structures, 1 for each lock present


//CONDITION PART
extern int nextConditionIndex;			//Increments each time a Condition is created. Sort of like a Condition ID to 
						//ensure a unique integer number is available for each Condition
extern Lock *kernelConditionTableLock;		//osConditions[] is the kernel Table for Conditions. This lock ensures mutual 
						//exclusion to this array
extern int MAX_CONDITIONS;			//Maximum number of Condition available

struct kernelCondition				//Structure for the Kernel Condition Table. Holds details for each Condition
{
	Condition *condition;
	AddrSpace *as;
				
	int usageCounter;			//Will be initialized to 0 in Create Lock system call
	bool toBeDestroyed;			
};

extern kernelCondition osConditions[];		//Array of structures, 1 for each lock present


// *****************CREATED ENDS HERE****************



extern Machine* machine;	// user program memory and registers
#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
#endif


#endif // SYSTEM_H
