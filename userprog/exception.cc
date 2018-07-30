// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <stdio.h>
#include <iostream>

using namespace std;

extern "C" {int bzero(char *, int); };

#ifdef NETWORK

struct db
{
	int machineID;
	int mailboxNO;
	int sequenceNo;

	long int timeStamp;
};

struct messageStruct
{
	int machineID;
	int mailboxNO;
	long int timeStamp;

	char message[MaxMailSize];
};








int MAXIMUM_WAITING = 20;
int lcounter = 0;        		//creating a lock counter for the number of locks
int ConditionCounter = 0;               //Counter for condition variable
char MVName[40];			
int mvcounter=1;
int MAX_MV=40;





struct waitQueue
{
    char replyMsg[40];
    int machineId;
    int mailboxNo;

};



struct mainserverLock
{
    char name[20];
    int myId;
    int status;     			 // Status is busy when it is 0 and it is free when its is 1
    int ownerMachineId;
    int ownerMailboxNo;
    waitQueue *waitingQueueLock[200];
    List *LockwaitingQueue;
    int waitingCount;
    int waitingCount1;
};

struct MonitorVariables
{
char name[3];
int myId;
int value;
};



struct mainserverCV
{
    	char name[20];
    	int CId;
    	int myLockID;
	int locksmacID;
	int LockMailboxNo;
	int waitingCount;
	int valid;
	int waitingArrayCtr;
	int waitingArrayCtr1;
	waitQueue waitingThread[20];
	List *WaitList;
};

MonitorVariables MV[200];

mainserverLock mainserverLocks[100];
mainserverCV mainserverCVs[100];



#endif		

int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n=0;			// The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
      result = machine->ReadMem( vaddr, 1, paddr );
      while(!result) // FALL 09 CHANGES
	  {
   		result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
	  }	
       
      buf[n++] = *paddr;
     
      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    delete paddr;
    return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes to the current thread's virtual address vaddr.
    // Return the number of bytes so written, or -1 if an error
    // occors.  Errors can generally mean a bad virtual address was
    // passed in.
    bool result;
    int n=0;			// The number of bytes copied in

    while ( n >= 0 && n < len) {
      // Note that we check every byte's address
      result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    return n;
}

void Create_Syscall(unsigned int vaddr, int len) {
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len+1];	// Kernel buffer to put the name in

    if (!buf) return;

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Create\n");
	delete buf;
	return;
    }

    buf[len]='\0';

    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

int Open_Syscall(unsigned int vaddr, int len) {
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    OpenFile *f;			// The new open file
    int id;				// The openfile id

    if (!buf) {
	printf("%s","Can't allocate kernel buffer in Open\n");
	return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Open\n");
	delete[] buf;
	return -1;
    }

    buf[len]='\0';

    f = fileSystem->Open(buf);
    delete[] buf;

    if ( f ) {
	if ((id = currentThread->space->fileTable.Put(f)) == -1 )
	    delete f;
	return id;
    }
    else
	return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one. For disk files, the file is looked
    // up in the current address space's open file table and used as
    // the target of the write.
    
    char *buf;		// Kernel buffer for output
    OpenFile *f;	// Open file for output

    if ( id == ConsoleInput) return;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer for write!\n");
	return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
	    printf("%s","Bad pointer passed to to write: data not written\n");
	    delete[] buf;
	    return;
	}
    }

    if ( id == ConsoleOutput) {
      for (int ii=0; ii<len; ii++) {
	printf("%c",buf[ii]);
      }

    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    f->Write(buf, len);
	} else {
	    printf("%s","Bad OpenFileId passed to Write\n");
	    len = -1;
	}
    }

    delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;		// Kernel buffer for input
    OpenFile *f;	// Open file for output

    if ( id == ConsoleOutput) return -1;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer in Read\n");
	return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
      scanf("%s", buf);

      if ( copyout(vaddr, len, buf) == -1 ) {
	printf("%s","Bad pointer passed to Read: data not copied\n");
      }
    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    len = f->Read(buf, len);
	    if ( len > 0 ) {
	        //Read something from the file. Put into user's address space
  	        if ( copyout(vaddr, len, buf) == -1 ) {
		    printf("%s","Bad pointer passed to Read: data not copied\n");
		}
	    }
	} else {
	    printf("%s","Bad OpenFileId passed to Read\n");
	    len = -1;
	}
    }

    delete[] buf;
    return len;
}

void Close_Syscall(int fd) {
    // Close the file associated with id fd.  No error reporting.
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

    if ( f ) {
      delete f;
    } else {
      printf("%s","Tried to close an unopen file\n");
    }
}

			//  *****************CREATED STARTS HERE*********************

void CreateMV_Syscall(unsigned int name, int size)
{
    #ifdef NETWORK

printf("CREATING MONITOR VARIABLE SYSTEM CALL\n");
      char* buf = new char[size+1];
        copyin(name,size,buf);
       
        PacketHeader outPktHdr, inPktHdr;		// initializing packet headers..
        MailHeader outMailHdr, inMailHdr;
	
        char *data="";					//data that has to be sent back.				
	
        char *ack = "Got it!";
        char buffer[MaxMailSize];
	printf("The Monitor Variable name is = %s\n ",buf);
         DEBUG('x', "In CreateMV\n");
		sprintf(data, "10@%d@%s",size, buf);	
	

        // construct packet, mail header for original message
        // To: destination machine, mailbox 0
        // From: our machine, reply to: mailbox 1
        outPktHdr.to = postOffice->GetMachineID();					//Hardcoding the server machine ID
    	outMailHdr.to =currentThread->mailboxNo;
    	outMailHdr.from = currentThread->mailboxNo-1;
        outMailHdr.length = strlen(data) + 1;

         //printf( "KKKKKKKKKKKKKKKKKKK %s\n", data);

        // Send the first message
        bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

        if ( !success ) 
        {
            printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
            interrupt->Halt();
        }

        //printf("Request Sent\n");
        
        // Wait for the first message from the other machine
        postOffice->Receive(currentThread->mailboxNo,&inPktHdr, &inMailHdr, buffer);
        //printf("%s\n",buffer);
         printf("CreatedMV:Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);

    #endif
}

void GetMV_Syscall(unsigned int name , int size)
{

    #ifdef NETWORK

printf("CREATING GET MONITOR VARIABLE SYSTEM CALL\n");
      char* buf = new char[size+1];
        copyin(name,size,buf);
       
        PacketHeader outPktHdr, inPktHdr;		// initializing packet headers..
        MailHeader outMailHdr, inMailHdr;
	
        char *data="";					//data that has to be sent back.				
	
        char *ack = "Got it!";
        char buffer[MaxMailSize];
	printf("The Monitor Variable name you want to get is = %s\n ",buf);
         DEBUG('x', "In GetMV\n");
		sprintf(data, "11@%d@%s",size, buf);



        // construct packet, mail header for original message
        // To: destination machine, mailbox 0
        // From: our machine, reply to: mailbox 1
        outPktHdr.to = postOffice->GetMachineID();					//Hardcoding the server machine ID
    	outMailHdr.to =currentThread->mailboxNo;
    	outMailHdr.from = currentThread->mailboxNo-1;
        outMailHdr.length = strlen(data) + 1;

        
        // Send the first message
        bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

        if ( !success ) 
        {
            printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
            interrupt->Halt();
        }

         // Wait for the first message from the other machine
        postOffice->Receive(currentThread->mailboxNo,&inPktHdr, &inMailHdr, buffer);
        printf(" The value of the monitor variable is %s\n",buffer);
        printf("GetMV:Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout); 
    #endif
}

void SetMV_Syscall(unsigned int name, int size, int value)
{
 #ifdef NETWORK

printf("CREATING GET MONITOR VARIABLE SYSTEM CALL\n");
      char* buf = new char[size+1];
        copyin(name,size,buf);
       
        PacketHeader outPktHdr, inPktHdr;		// initializing packet headers..
        MailHeader outMailHdr, inMailHdr;
	
        char *data="";					//data that has to be sent back.				
	
        char *ack = "Got it!";
        char buffer[MaxMailSize];
	printf("The Monitor Variable name you want to set is = %s and the value to which = %d\n ",buf,value);
         DEBUG('x', "In GetMV\n");
		sprintf(data, "12@%d@%s@%d",size, buf,value);



        // construct packet, mail header for original message
        // To: destination machine, mailbox 0
        // From: our machine, reply to: mailbox 1
        outPktHdr.to = postOffice->GetMachineID();					//Hardcoding the server machine ID
    	outMailHdr.to =currentThread->mailboxNo;
    	outMailHdr.from = currentThread->mailboxNo-1;
        outMailHdr.length = strlen(data) + 1;

         
        // Send the first message
        bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

        if ( !success ) 
        {
            printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
            interrupt->Halt();
        }

        
        
        // Wait for the first message from the other machine
        memset(buffer,'\0',MaxMailSize);
        postOffice->Receive(currentThread->mailboxNo,&inPktHdr, &inMailHdr, buffer);
        printf("SET Completed :%s\n",buffer);
        printf("SETMV:Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);

    #endif

}

void IncrementMV_Syscall(unsigned int name, int size)
{
 #ifdef NETWORK

printf("CREATING INCREMENT MONITOR VARIABLE SYSTEM CALL\n");
      char* buf = new char[size+1];
        copyin(name,size,buf);
       
        PacketHeader outPktHdr, inPktHdr;		// initializing packet headers..
        MailHeader outMailHdr, inMailHdr;
	
        char *data="";					//data that has to be sent back.				
	
        char *ack = "Got it!";
        char buffer[MaxMailSize];
	printf("The Monitor Variable name you want to get is = %s\n ",buf);
         DEBUG('x', "In GetMV\n");
		sprintf(data, "13@%d@%s",size, buf);


        // construct packet, mail header for original message
        // To: destination machine, mailbox 0
        // From: our machine, reply to: mailbox 1
        outPktHdr.to = postOffice->GetMachineID();					//Hardcoding the server machine ID
    	outMailHdr.to =currentThread->mailboxNo;
    	outMailHdr.from = currentThread->mailboxNo-1;
        outMailHdr.length = strlen(data) + 1;

         //printf( "KKKKKKKKKKKKKKKKKKK %s\n", data);

        // Send the first message
        bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

        if ( !success ) 
        {
            printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
            interrupt->Halt();
        }

        
        // Wait for the first message from the other machine
        postOffice->Receive(currentThread->mailboxNo,&inPktHdr, &inMailHdr, buffer);
        printf("INCREMENT :%s\n",buffer);
        printf("INCREMENTCV:Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);

    #endif

}

void DecrementMV_Syscall(unsigned int name, int size)
{

 #ifdef NETWORK

printf("CREATING DECREMENT MONITOR VARIABLE SYSTEM CALL\n");
      char* buf = new char[size+1];
        copyin(name,size,buf);
       
        PacketHeader outPktHdr, inPktHdr;		// initializing packet headers..
        MailHeader outMailHdr, inMailHdr;
	
        char *data="";					//data that has to be sent back.				
	
        char *ack = "Got it!";
        char buffer[MaxMailSize];
	printf("The Monitor Variable name you want to decrement is = %s\n ",buf);
         DEBUG('x', "In GetMV\n");
		sprintf(data, "14@%d@%s",size, buf);


        // construct packet, mail header for original message
        // To: destination machine, mailbox 0
        // From: our machine, reply to: mailbox 1
        outPktHdr.to = postOffice->GetMachineID();					//Hardcoding the server machine ID
    	outMailHdr.to =currentThread->mailboxNo;
    	outMailHdr.from = currentThread->mailboxNo-1;
        outMailHdr.length = strlen(data) + 1;

         //printf( "KKKKKKKKKKKKKKKKKKK %s\n", data);

        // Send the first message
        bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

        if ( !success ) 
        {
            printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
            interrupt->Halt();
        }

        //printf("Request Sent\n");
        
        // Wait for the first message from the other machine
        postOffice->Receive(currentThread->mailboxNo,&inPktHdr, &inMailHdr, buffer);
        
        printf("DECREMENT:Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);

    #endif
}


void DMV_Syscall(unsigned int name, int size)
{

#ifdef NETWORK
	printf("CREATING DECREMENT MONITOR VARIABLE SYSTEM CALL\n");
      	char* buf = new char[size+1];
        copyin(name,size,buf);
       
        PacketHeader outPktHdr, inPktHdr;		// initializing packet headers..
        MailHeader outMailHdr, inMailHdr;
	
        char *data="";					//data that has to be sent back.				
	
        char *ack = "Got it!";
        char buffer[MaxMailSize];
	printf("The Monitor Variable name you want to delete is = %s\n ",buf);
         DEBUG('x', "In GetMV\n");
		sprintf(data, "15@%d@%s",size, buf);


        // construct packet, mail header for original message
        // To: destination machine, mailbox 0
        // From: our machine, reply to: mailbox 1
        outPktHdr.to = postOffice->GetMachineID();					//Hardcoding the server machine ID
    	outMailHdr.to =currentThread->mailboxNo;
    	outMailHdr.from = currentThread->mailboxNo-1;
        outMailHdr.length = strlen(data) + 1;

         DEBUG('x', "In DestroyMV\n");
	
        // Send the first message
        bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

        if ( !success ) 
        {
            printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
            interrupt->Halt();
        }

        printf("Request Sent\n");
        
        // Wait for the first message from the other machine
        postOffice->Receive(currentThread->mailboxNo, &inPktHdr, &inMailHdr, buffer);
        printf("DestroyMV:Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);
#endif
}


//Implementation of System Calls for Locks

int CreateLock_Syscall(unsigned int name, int size)
{
#ifdef NETWORK

printf("\nCREATING NETWORK CREATE LOCK SYSTEM CALL\n");
	
        if(size <= 0 || size > 30)                       //Checking  the length of the size of the name
        {
            DEBUG('x', "The name is too big, Error!!! size %d name %d\n", size , name);
            return -1;
        }

	char* buf = new char[size+1];
        copyin(name,size,buf);
       
        PacketHeader outPktHdr, inPktHdr;		// initializing packet headers..
        MailHeader outMailHdr, inMailHdr;
	
        char *data="";					//data that has to be sent back.				
	
        char *ack = "Got it!";
        char buffer[MaxMailSize];
	printf("The lock name is = %s\n ",buf);
         DEBUG('x', "In CreateLock\n");
		sprintf(data, "1@%d@%s",size, buf);	
	
	//printf("\n      %s     \n ",data);

        // construct packet, mail header for original message
        // To: destination machine, mailbox 0
        // From: our machine, reply to: mailbox 1
         outPktHdr.to = postOffice->GetMachineID();					//Hardcoding the server machine ID
    	outMailHdr.to =currentThread->mailboxNo;
    	outMailHdr.from = currentThread->mailboxNo-1;
        outMailHdr.length = strlen(data) + 1;

         DEBUG('x', "In CreateLock\n");

        // Send the first message


        bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

        if ( !success ) 
        {
            printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
            interrupt->Halt();
        }

        printf("Request Sent\n");
	
        
        // Wait for the first message from the other machine

        postOffice->Receive(currentThread->mailboxNo, &inPktHdr, &inMailHdr, buffer);
        printf("CreateLock:Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);

     

        int lockId = atoi(buffer);			//Getting the lock ID.
        
        if(lockId == -1)
        {
            DEBUG('x', "Error in Creating Lock\n");
            interrupt->Halt();
            return lockId;
        }
        else
        {
            return lockId;
        }


#else	
	int indexOfNewLock;


	char *tempLockName;			
	char *buf = new char[size+1];	// Kernel buffer to put the name in

    	if (!buf) return -1;

    	if( copyin(name,size,buf) == -1 ) 
	{
		DEBUG('x', "Bad pointer passed\n");
		delete buf;
		return -1;
    	}

    	buf[size]='\0';


	kernelLockTableLock -> Acquire();					//KernelLockTableLock size Acquired now
										//for accessing osLocks[]
		
	if (nextLockIndex >= MAX_LOCKS)						//Check that the Table is not full
	{
		DEBUG('x', " Locks Table is full now, Error!!\n");
	
		kernelLockTableLock -> Release();	
	
		return -1;
	}
	

	//Setting Structure kernelLock's values
	osLocks[nextLockIndex].lock=new Lock(buf);				//Lock's name is set now

	osLocks[nextLockIndex].as=currentThread -> space;			//Lock is using the Current Thread's space

	osLocks[nextLockIndex].usageCounter=0;					//Will be set to 1, when the Lock is being used, 
										//i.e. the lock was acquired
	osLocks[nextLockIndex].toBeDestroyed=FALSE;		


	tempLockName=osLocks[nextLockIndex].lock -> getName();
	DEBUG('x', "\nLock No: %d Name: %s created \n\n", nextLockIndex, tempLockName);

	indexOfNewLock=nextLockIndex;
	nextLockIndex++;							//incremented after the current index
										//has been assigned
	kernelLockTableLock -> Release();

	return indexOfNewLock;							//Integer for user returned
#endif

}


void DestroyLock_Syscall(int indexOfDel)
{

#ifdef NETWORK

printf("CREATING NETWORK DESTROY LOCK SYSTEM CALL\n");
        PacketHeader outPktHdr, inPktHdr;
        MailHeader outMailHdr, inMailHdr;
        char *data = "";
        char *ack = "Got it!";
        char buffer[MaxMailSize];
		
         DEBUG('x', "In DestroyLock\n");
		
	sprintf(data, "2@%d",indexOfDel);

        // construct packet, mail header for original message
        // To: destination machine, mailbox 0
        // From: our machine, reply to: mailbox 1
        outPktHdr.to = postOffice->GetMachineID();					//Hardcoding the server machine ID
    	outMailHdr.to =currentThread->mailboxNo;
    	outMailHdr.from = currentThread->mailboxNo-1;
        outMailHdr.length = strlen(data) + 1;

         DEBUG('x', "In DestroyLock\n");
	printf("The lock that is destroyed is = %d \n ",indexOfDel);
        // Send the first message
        bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

        if ( !success ) 
        {
            printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
            interrupt->Halt();
        }

        printf("Request Sent\n");
        
        // Wait for the first message from the other machine
        postOffice->Receive(currentThread->mailboxNo, &inPktHdr, &inMailHdr, buffer);
        printf("DestroyLock:Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);

     




#else
	char *tempLockName;		

	
	kernelLockTableLock -> Acquire();
	
	if (osLocks[indexOfDel].lock == NULL || indexOfDel < 0 || indexOfDel >= nextLockIndex)
	{
		DEBUG('x', "Invalid index passed as parameter. Error !!!\n");
		
		kernelLockTableLock -> Release();

		return;
	}
	
	
	tempLockName=osLocks[indexOfDel].lock -> getName();
	

	if (osLocks[indexOfDel].usageCounter > 0)
	{
		DEBUG('x', "Lock No: %d, Name: %s is in use. Error !!!\n", indexOfDel, tempLockName);
		osLocks[indexOfDel].toBeDestroyed=TRUE;

		kernelLockTableLock -> Release();

		return;
	}

	if (osLocks[indexOfDel].usageCounter == 0  ||  osLocks[indexOfDel].toBeDestroyed == TRUE)
	{
		osLocks[indexOfDel].toBeDestroyed=TRUE;

		osLocks[indexOfDel].lock=NULL;					//Set to NULL to prevent Segmentation Fault
	
		
		DEBUG('x', "\nLock No: %d, Name: %s destroyed \n\n", indexOfDel, tempLockName);
	}

	

	kernelLockTableLock -> Release();
#endif
}	

void Acquire_Syscall(int indexOfAcq)
{

#ifdef NETWORK
        printf("CREATING NETWORK ACQUIRE SYSTEM CALL\n");
        
        PacketHeader outPktHdr, inPktHdr;
        MailHeader outMailHdr, inMailHdr;
        char *data = "";
        char *ack = "Got it!";
        char buffer[MaxMailSize];
		
	sprintf(data, "3@%d",indexOfAcq);

	printf("The index of lock to be acquired is = %d \n",indexOfAcq);

        // construct packet, mail header for original message
        // To: destination machine, mailbox 0
        // From: our machine, reply to: mailbox 1
        outPktHdr.to = postOffice->GetMachineID();					//Hardcoding the server machine ID
    	outMailHdr.to =currentThread->mailboxNo;
    	outMailHdr.from = currentThread->mailboxNo-1;
        outMailHdr.length = strlen(data) + 1;


        // Send the first message

        bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

        if ( !success ) 
        {
            printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
            interrupt->Halt();
        }


        
        // Wait for the first message from the other machine

        postOffice->Receive(currentThread->mailboxNo, &inPktHdr, &inMailHdr, buffer);
        printf("Acquire:Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);

        int lockId = atoi(buffer);
        if(lockId == -1)
        {
            DEBUG('x', "Error in Acquiring Lock\n");
            interrupt->Halt();
        }
        
        return;

    #else

	char *tempLockName;
	
	kernelLockTableLock -> Acquire();


	if (indexOfAcq < 0  ||  indexOfAcq >= nextLockIndex)
	{
		DEBUG('x', "Invalid index passed as parameter. Error !!!\n");

		kernelLockTableLock -> Release();

		return;
	}


	if (osLocks[indexOfAcq].lock == NULL)
	{
		DEBUG('x', "Lock doesn't exist. Error !!!\n");

		kernelLockTableLock -> Release();

		return;
	}

	if (osLocks[indexOfAcq].as != currentThread -> space)		//Lock belongs to a different process
	{
		DEBUG('x', "Lock belongs to a different process. Error !!!\n");
	
		kernelLockTableLock -> Release();

		return;
	}

	

	tempLockName=osLocks[indexOfAcq].lock -> getName();

	
	DEBUG('x', "\nLock No: %d, Name: %s Acquired \n\n", indexOfAcq, tempLockName);
	
	osLocks[indexOfAcq].usageCounter++;
	kernelLockTableLock -> Release();

	osLocks[indexOfAcq].lock -> Acquire();
#endif
}




void Release_Syscall(int indexOfRel)
{

#ifdef NETWORK

        printf("CREATING NETWORK RELEASE SYSTEM CALL\n");
        
        PacketHeader outPktHdr, inPktHdr;
        MailHeader outMailHdr, inMailHdr;
        char *data = "";
        char *ack = "Got it!";
        char buffer[MaxMailSize];
		
	sprintf(data, "4@%d",indexOfRel);

        // construct packet, mail header for original message
        // To: destination machine, mailbox 0
        // From: our machine, reply to: mailbox 1
        outPktHdr.to = postOffice->GetMachineID();					//Hardcoding the server machine ID
    	outMailHdr.to =currentThread->mailboxNo;
    	outMailHdr.from = currentThread->mailboxNo-1;
        outMailHdr.length = strlen(data) + 1;

	printf("The lock that is being released has id = %d\n",indexOfRel);
        // Send the first message
        bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

        if ( !success ) 
        {
            printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
            interrupt->Halt();
        }


        
        // Wait for the first message from the other machine

        postOffice->Receive(currentThread->mailboxNo, &inPktHdr, &inMailHdr, buffer);
        printf("Release:Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);

        int lockId = atoi(buffer);
        if(lockId == -1)
        {
            DEBUG('x', "Error in Releasing Lock\n");
            interrupt->Halt();
        }
        
        
        return;


    #else

	char *tempLockName;	

	kernelLockTableLock -> Acquire();

	if (indexOfRel < 0  ||  indexOfRel >= nextLockIndex)
	{
		DEBUG('x', "Invalid index passed as parameter. Error !!!\n");
		
		kernelLockTableLock -> Release();

		return;
	}

	
	if (osLocks[indexOfRel].lock == NULL)
	{
		DEBUG('x', "Lock doesn't exist. Error !!!\n");
		
		kernelLockTableLock -> Release();

		return;
	}

	if (osLocks[indexOfRel].as != currentThread -> space)		//Lock belongs to a different process
	{
		DEBUG('x', "Lock belongs to a different process. Error !!!\n");
		
		kernelLockTableLock -> Release();

		return;
	}

	tempLockName=osLocks[indexOfRel].lock -> getName();


	osLocks[indexOfRel].usageCounter--;
	osLocks[indexOfRel].lock -> Release();
	
	DEBUG('x',"\nLock No: %d, Name: %s released \n\n", indexOfRel, tempLockName);

	kernelLockTableLock -> Release();
#endif

}



//Implementation of System Calls for Condition Variables

int CreateCondition_Syscall(unsigned int name, int size)
{

#ifdef NETWORK
	printf("CREATING NETWORK CREATE CONDITION SYSTEM CALL\n");
        if(size <= 0 || size > 30)                       //Checking  the length of the size of the name
        {
            DEBUG('x', "The name is too big, Error!!! size %d name %d\n", size , name);
            return -1;
        }
        char* buf = new char[size+1];
        copyin(name,size,buf);
        
        PacketHeader outPktHdr, inPktHdr;
        MailHeader outMailHdr, inMailHdr;
        char *data = "";
        char *ack = "Got it!";
        char buffer[MaxMailSize];
		
        DEBUG('x', "In CreateCondition\n");
	sprintf(data, "5@%d@%s",size, buf);

        // construct packet, mail header for original message
        // To: destination machine, mailbox 0
        // From: our machine, reply to: mailbox 1
        outPktHdr.to = postOffice->GetMachineID();					//Hardcoding the server machine ID
    	outMailHdr.to =currentThread->mailboxNo;
    	outMailHdr.from = currentThread->mailboxNo-1;
        outMailHdr.length = strlen(data) + 1;

         DEBUG('x', "In CreateCondition\n");

        // Send the first message
        bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

        if ( !success ) 
        {
            printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
            interrupt->Halt();
        }

        printf("Request Sent\n");
        
        // Wait for the first message from the other machine
        postOffice->Receive(currentThread->mailboxNo, &inPktHdr, &inMailHdr, buffer);
        printf("CreateCondition:Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);

     

        int lockId = atoi(buffer);			//Getting the lock ID.
        
        if(lockId == -1)
        {
            DEBUG('x', "Error in Creating Condition\n");
            interrupt->Halt();
            return lockId;
        }
        else
        {
            return lockId;
        }

#else

	int indexOfNewLock;
	int indexOfNewCondition;

	char *tempConditionName;
	char *buf = new char[size+1];	// Kernel buffer to put the name in

    	if (!buf) return -1;

    	if( copyin(name,size,buf) == -1 ) 
	{
		DEBUG('x',"Bad pointer passed\n");
		delete buf;
		return -1;
    	}

    	buf[size]='\0';


	kernelConditionTableLock -> Acquire();					//KernelConditionTableLock size Acquired now
										//for accessing osConditions[]
				
	if (nextConditionIndex >= MAX_CONDITIONS)				//Check that the Table is not full
	{
		DEBUG('x', " Conditions Table is full now, Error!!\n");
	
		kernelConditionTableLock -> Release();	
	
		return -1;
	}

	
	//Setting Structure kernelCondition's values
	osConditions[nextConditionIndex].condition=new Condition(buf);			//Condition's name is set now
	osConditions[nextConditionIndex].as=currentThread -> space;			//Condition is using the Current Thread's 
											//space
	osConditions[nextConditionIndex].usageCounter=0;				//Will be set to 1, when the Condition 
											//is being used, 
											//i.e. the lock was acquired
	osConditions[nextConditionIndex].toBeDestroyed=FALSE;		

	
	tempConditionName=osConditions[nextConditionIndex].condition -> getName();
	DEBUG ('x',"\nCondition No: %d Name: %s created \n\n", nextConditionIndex, tempConditionName);
			
	indexOfNewCondition=nextConditionIndex;
	nextConditionIndex++;								//incremented after the current index
											//has been assigned

	kernelConditionTableLock -> Release();


	return indexOfNewCondition;							//Integer for user returned


#endif

}	



void DestroyCondition_Syscall(int indexOfDel)
{


#ifdef NETWORK
        PacketHeader outPktHdr, inPktHdr;
        MailHeader outMailHdr, inMailHdr;
        char *data = "";
        char *ack = "Got it!";
        char buffer[MaxMailSize];
 
        printf("CREATING THE SYSTEM CALL FOR DESTROY CONDITION\n");
		sprintf(data, "6@%d",indexOfDel);

        // construct packet, mail header for original message
        // To: destination machine, mailbox 0
        // From: our machine, reply to: mailbox 1
        outPktHdr.to = postOffice->GetMachineID();					//Hardcoding the server machine ID
    	outMailHdr.to =currentThread->mailboxNo;
    	outMailHdr.from = currentThread->mailboxNo-1;
        outMailHdr.length = strlen(data) + 1;

         DEBUG('x', "In DestroyCondition\n");

        // Send the first message
        bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

        if ( !success ) 
        {
            printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
            interrupt->Halt();
        }

        printf("Request Sent\n");
        
        // Wait for the first message from the other machine

        postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
	DEBUG('x', "my machine id = %d and mailbox no = %d\n",inPktHdr.to, inMailHdr.to);
        printf("DestroyCondition:Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);

        int CndvarID = atoi(buffer);
        
        if(CndvarID == -1)
        {
            DEBUG('x', "Error in Destroy Condition\n");
            interrupt->Halt();
        }
        
    #else

	char *tempConditionName;	
	
	kernelConditionTableLock -> Acquire();
	
	if (osConditions[indexOfDel].condition == NULL || indexOfDel < 0  ||  indexOfDel >= nextConditionIndex)
	{
		DEBUG('x', "Invalid index passed as parameter. Error !!!\n");
		
		kernelConditionTableLock -> Release();

		return;
	}


	tempConditionName=osConditions[indexOfDel].condition -> getName();


	if (osConditions[indexOfDel].usageCounter > 0)
	{
		DEBUG('x',"Condition No: %d, Name: %s is in use. Error !!!\n", indexOfDel, tempConditionName);
		osConditions[indexOfDel].toBeDestroyed=TRUE;

		kernelConditionTableLock -> Release();

		return;
	}

	if (osConditions[indexOfDel].usageCounter == 0  ||  osConditions[indexOfDel].toBeDestroyed == TRUE)
	{
		osConditions[indexOfDel].toBeDestroyed=TRUE;

		delete osConditions[indexOfDel].condition;				//Deallocated Condition
		osConditions[indexOfDel].condition=NULL;				//Set to NULL to prevent Segmentation Fault

		
		DEBUG('x',"\nCondition No: %d, Name: %s destroyed \n\n", indexOfDel, tempConditionName);	

	}


	kernelConditionTableLock -> Release();
#endif

}	


void Wait_Syscall(int indexCondition, int indexLock)
{
	
 #ifdef NETWORK
        
        PacketHeader outPktHdr, inPktHdr;
        MailHeader outMailHdr, inMailHdr;
        char *data = "";
        char *ack = "Got it!";
        char buffer[MaxMailSize];
		
        printf("Waiting for a Signal\n");
	sprintf(data, "7@%d@%d",indexLock, indexCondition);

        // construct packet, mail header for original message
        // To: destination machine, mailbox 0
        // From: our machine, reply to: mailbox 1
        outPktHdr.to = postOffice->GetMachineID();					//Hardcoding the server machine ID
        outMailHdr.to =currentThread->mailboxNo;
        outMailHdr.from = currentThread->mailboxNo-1;
        outMailHdr.length = strlen(data) + 1;

         DEBUG('x', "In Waiting Condition Variable\n");

        // Send the first message
        bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

        if ( !success ) 
        {
           printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
            interrupt->Halt();
        }

        printf("Request Sent\n");
        
        // Wait for the first message from the other machine

	
        postOffice->Receive(currentThread->mailboxNo, &inPktHdr, &inMailHdr, buffer);
        printf("Wait:Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
	
        fflush(stdout);
        



        int CondvarID = atoi(buffer);
        
        if(CondvarID == -1)
        {
            printf("Error in Wait System call\n");
            interrupt->Halt();
        }
        

    #else

char *tempConditionName, *tempLockName;

	kernelConditionTableLock -> Acquire();

	if (indexCondition < 0  ||  indexCondition >= nextConditionIndex)
	{
		DEBUG('x', "Invalid index passed as parameter. Error !!!\n");

		kernelConditionTableLock -> Release();

		return;
	}


	if (osConditions[indexCondition].condition == NULL)
	{
		DEBUG('x',"Condition doesn't exist. Error !!!\n");

		kernelConditionTableLock -> Release();

		return;
	}

	if (osConditions[indexCondition].as != currentThread -> space)		//Condition belongs to a different process
	{
		DEBUG('x',"Condition belongs to a different process. Error !!!\n");
	
		kernelConditionTableLock -> Release();

		return;
	}

		
	osConditions[indexCondition].usageCounter++;
	
	tempConditionName=osConditions[indexCondition].condition -> getName();


	kernelConditionTableLock -> Release();


	kernelLockTableLock -> Acquire();

	if (indexLock < 0  ||  indexLock >= nextLockIndex) 
	{
		DEBUG('x',"Invalid index passed as parameter. Error !!!\n");

		kernelLockTableLock -> Release();

		return;
	}


	if (osLocks[indexLock].lock == NULL)
	{
		DEBUG('x',"Lock doesn't exist. Error !!!\n");

		kernelLockTableLock -> Release();

		return;
	}

	if (osLocks[indexLock].as != currentThread -> space)		//Lock belongs to a different process
	{
		DEBUG('x',"Lock belongs to a different process. Error !!!\n");
	
		kernelLockTableLock -> Release();

		return;
	}

		
	osLocks[indexLock].usageCounter++;

	tempLockName=osLocks[indexLock].lock -> getName();

	
	kernelLockTableLock -> Release();
	
	
	DEBUG('x',"\nCondition No: %d, Name: %s waiting on Lock No: %d, Name: %s\n\n", indexCondition, tempConditionName, indexLock, tempLockName);


	osConditions[indexCondition].condition -> Wait(osLocks[indexLock].lock);
#endif
}


void Signal_Syscall(int indexCondition, int indexLock)
{


#ifdef NETWORK
        PacketHeader outPktHdr, inPktHdr;
        MailHeader outMailHdr, inMailHdr;
        char *data = "";
        char *ack = "Got it!";
        char buffer[MaxMailSize];
		
         printf("CREATING SIGNAL WAIT SYSTEM CALL\n");
	sprintf(data, "8@%d@%d",indexLock, indexCondition);

        // construct packet, mail header for original message
        // To: destination machine, mailbox 0
        // From: our machine, reply to: mailbox 1
        


	outPktHdr.to = postOffice->GetMachineID();					//Hardcoding the server machine ID
        outMailHdr.to =currentThread->mailboxNo;
        outMailHdr.from = currentThread->mailboxNo-1;
        outMailHdr.length = strlen(data) + 1;

         DEBUG('x', "In signal system call\n");

        // Send the first message
        bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

        if ( !success ) 
        {
            printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
            interrupt->Halt();
        }

        printf("Request Sent\n");
        
        // Wait for the first message from the other machine
        postOffice->Receive(currentThread->mailboxNo, &inPktHdr, &inMailHdr, buffer);
        printf("Signal:Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);

   
        int CondvarID = atoi(buffer);
        
        if(CondvarID == -1)
        {
            DEBUG('x', "Error in Signal system call\n");
            interrupt->Halt();
        }
        

    #else

	char *tempConditionName, *tempLockName;


	kernelConditionTableLock -> Acquire();

	if (indexCondition < 0  ||  indexCondition >= nextConditionIndex)
	{
		DEBUG('x',"Invalid index passed as parameter. Error !!!\n");

		kernelConditionTableLock -> Release();

		return;
	}



	if (osConditions[indexCondition].condition == NULL)
	{
		DEBUG('x',"Condition doesn't exist. Error !!!\n");

		kernelConditionTableLock -> Release();

		return;
	}

	if (osConditions[indexCondition].as != currentThread -> space)		//Condition belongs to a different process
	{
		DEBUG('x',"Condition belongs to a different process. Error !!!\n");
	
		kernelConditionTableLock -> Release();

		return;
	}

	tempConditionName=osConditions[indexCondition].condition -> getName();

		
	osConditions[indexCondition].usageCounter--;
	
	kernelConditionTableLock -> Release();



	kernelLockTableLock -> Acquire();

	if (indexLock < 0  ||  indexLock >= nextLockIndex)
	{
		DEBUG('x',"Invalid index passed as parameter. Error !!!\n");

		kernelLockTableLock -> Release();

		return;
	}


	if (osLocks[indexLock].lock == NULL)
	{
		DEBUG('x',"Lock doesn't exist. Error !!!\n");

		kernelLockTableLock -> Release();

		return;
	}

	if (osLocks[indexLock].as != currentThread -> space)		//Lock belongs to a different process
	{
		DEBUG('x',"Lock belongs to a different process. Error !!!\n");
	
		kernelLockTableLock -> Release();

		return;
	}

		
	tempLockName=osLocks[indexLock].lock -> getName();

	osLocks[indexLock].usageCounter--;
	
	DEBUG('x',"\nCondition No: %d, Name: %s signalling Lock No: %d, Name: %s\n\n", indexCondition, tempConditionName, indexLock, tempLockName);
	osConditions[indexCondition].condition -> Signal(osLocks[indexLock].lock);

	kernelLockTableLock -> Release();
#endif
}


void Broadcast_Syscall(int indexCondition, int indexLock)
{

 #ifdef NETWORK
        PacketHeader outPktHdr, inPktHdr;
        MailHeader outMailHdr, inMailHdr;
        char *data = "";
        char *ack = "Got it!";
        char buffer[MaxMailSize];
		
        DEBUG('x', "Creating Broadcast system call\n");
	sprintf(data, "9@%d@%d",indexLock, indexCondition);

        // construct packet, mail header for original message
        // To: destination machine, mailbox 0
        // From: our machine, reply to: mailbox 1
        outPktHdr.to = postOffice->GetMachineID();					//Hardcoding the server machine ID
        outMailHdr.to =currentThread->mailboxNo;
        outMailHdr.from = currentThread->mailboxNo-1;
        outMailHdr.length = strlen(data) + 1;

         DEBUG('x', "In Broadcast condition variable system call\n");

        // Send the first message
        bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

        if ( !success ) 
        {
            printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
            interrupt->Halt();
        }

        printf("Request Sent\n");
        
        // Wait for the first message from the other machine
        postOffice->Receive(currentThread->mailboxNo, &inPktHdr, &inMailHdr, buffer);
        printf("Broadcast:Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);

        
        int CondvarID = atoi(buffer);
        
        if(CondvarID == -1)
        {
            DEBUG('x', "Error in Broadcast\n");
            interrupt->Halt();
        }
        

    #else

	int tempConditionUsageCounter;

	kernelConditionTableLock -> Acquire();

	if (indexCondition < 0  ||  indexCondition >= nextConditionIndex)
	{
		DEBUG('x',"Invalid index passed as parameter. Error !!!\n");

		kernelConditionTableLock -> Release();

		return;
	}



	if (osConditions[indexCondition].condition == NULL)
	{
		DEBUG('x',"Condition doesn't exist. Error !!!\n");

		kernelConditionTableLock -> Release();

		return;
	}

	if (osConditions[indexCondition].as != currentThread -> space)		//Condition belongs to a different process
	{
		DEBUG('x',"Condition belongs to a different process. Error !!!\n");
	
		kernelConditionTableLock -> Release();

		return;
	}

		
	tempConditionUsageCounter=osConditions[indexCondition].usageCounter;
	osConditions[indexCondition].usageCounter--;
	

	kernelConditionTableLock -> Release();


	kernelLockTableLock -> Acquire();

	if (indexLock < 0  ||  indexLock >= nextLockIndex)
	{	DEBUG('x',"Invalid index passed as parameter. Error !!!\n");

		kernelLockTableLock -> Release();

		return;
	}


	if (osLocks[indexLock].lock == NULL)
	{
		DEBUG('x',"Lock doesn't exist. Error !!!\n");

		kernelLockTableLock -> Release();

		return;
	}

	if (osLocks[indexLock].as != currentThread -> space)		//Lock belongs to a different process
	{
		DEBUG('x',"Lock belongs to a different process. Error !!!\n");
	
		kernelLockTableLock -> Release();

		return;
	}

		
	if(osConditions[indexCondition].usageCounter - tempConditionUsageCounter >= 0)
		osConditions[indexCondition].usageCounter = osLocks[indexLock].usageCounter - tempConditionUsageCounter;
  	else
    		osLocks[indexLock].usageCounter = 0;
  

	osConditions[indexCondition].condition -> Broadcast(osLocks[indexLock].lock);

	
	kernelLockTableLock -> Release();

#endif
}



void kernel_thread_function(unsigned int vaddr)
{
	forkInitializationLock -> Acquire();


	//Update the Process Table for Multiprogramming part
	currentThread -> space -> updatePageTable();			//Current Page Table replaced by one which is bigger than
									//the current paget table by 8 pages. numpages etc. refreshed

	machine -> WriteRegister(PCReg, vaddr);		                //Set The PC RC Register and the NextPCRegister
	

	//write  virtualaddress + 4 in NextPCReg
	machine -> WriteRegister(NextPCReg, vaddr + 4);
	

	//call  Restorestate function inorder  to prevent information loss while context switching
	currentThread -> space -> RestoreState();


	//write  to the stack register , the starting postion of the stack for this thread
	machine -> WriteRegister(StackReg, PageSize * (currentThread -> space -> numPages) - 16);
									//Set The stack pointer to top of the stack of this thread.

	forkInitializationLock -> Release();

	
	//call machine->Run() 
	machine -> Run();						

}
	

void Fork_Syscall(unsigned int vaddr)
{
	
//	printf("Entering Fork");

	pInfoLock -> Acquire();


	//Create a New thread. This would be a kernel thread
	Thread *kernelThread;
	kernelThread=new Thread("Kernel Thread");			//Thread to be forked


	pInfoTable.noThreads[currentThread -> processID]++;		//Number of threads in the current process +=1

	kernelThread -> processID=currentThread -> processID;		//kernelThread is run in the process's addrSpace
	

	//Allocate the addrespace to the thread being forked which is essentially current thread's addresspsace  
	//because threads share the process addressspace. 
	kernelThread -> space=currentThread -> space;  

	pInfoLock -> Release();				

	kernelThread -> Fork((VoidFunctionPtr)kernel_thread_function, vaddr);		//Runs kernel_thread_function in 
																				//kernel level fork()
}



#ifdef NETWORK


int Signal_RPC(int CondvarId,int lockId, int clientMachineId, int clientMailboxNo)
{

	PacketHeader outPktHdr;			// Packet header object for network side
        MailHeader outMailHdr;			// Mail header object for network side
    	

    	char *ack = "Got it!";

	outPktHdr.to = clientMachineId;		//sending packet to the client's machine id
        outMailHdr.to = clientMailboxNo;	//sending packet to the client's mailbox number


	


                // Checking if the condition is valid and the condition exits..




                if (CondvarId<0 || CondvarId > MAX_CONDITIONS)
                {
                	printf("COndition is not valid\n");
    				return -1;
                }
                else if (mainserverCVs[CondvarId].valid != 1)
                {
                	printf("COndition is not valid as it has already been deleted\n");
    				return -1;
                }
                




		//Getting the client Machine ID and Mailbox Number.

                //Checking if lock exists and is valid...



				
                else if (mainserverLocks[lockId].myId == -1 || lcounter == 0 || lockId > lcounter)
                {
                	printf("The Lock Does not exist\n");
    				return -1;
                }




                // Checking if the one who is signaling is the lock owner or not!




                else if (mainserverLocks[lockId].ownerMachineId != clientMachineId || mainserverLocks[lockId].ownerMailboxNo != clientMailboxNo)
                {
                	 DEBUG('x', "your machine id %d != %d or mailBoxNumber %d != %d\n Returning\n",mainserverLocks[lockId].ownerMachineId,clientMachineId,mainserverLocks[lockId].ownerMailboxNo,clientMailboxNo);
    				return -1;
                }
				
		

		
		else{

			 DEBUG('x', "now signalling mac = %d and mail = %d",mainserverCVs[CondvarId].waitingThread[mainserverCVs[CondvarId].waitingArrayCtr].machineId,mainserverCVs[CondvarId].waitingThread[mainserverCVs[CondvarId].waitingArrayCtr].mailboxNo);
//changed	
		
					mainserverCVs[CondvarId].waitingArrayCtr++;
	//change ends
				sprintf(ack, "%d", 1);


				}
				return 1;
					
				
}







void Acquire_RPC(int lockId, int clientMachineId, int clientMailboxNo)
{


	
    		PacketHeader outPktHdr;		// Packet header object for network side
   		MailHeader outMailHdr;		// Mail header object for network side



   		char *ack = "Got it!";		//using this ack msg to check if the lock has been acquired.
    
		bool success; 			//using this to check if the message is sent correctly over the network.
 
    		int MAX_PARAMETER = 10;         // There should be maximum 10 comma seperated parameters in the request message.
    

		char *request[MAX_PARAMETER];	//character array that will contain the request message.




    		Lock* mainserverLock=new Lock("mainserverLock");	//Server Lock 

   		outPktHdr.to = clientMachineId;			//sending packet to the client's machine id
        	outMailHdr.to = clientMailboxNo;		//sending packet to the client's mailbox number

   		
    		mainserverLock->Acquire();

    		if(mainserverLocks[lockId].status == 1)		//checking the status of the lock.
    		{
    
    
        
        	mainserverLocks[lockId].status = 0;				//setting the lock status to busy.

		
        	mainserverLocks[lockId].ownerMachineId = clientMachineId;	//setting the lock ownership i.e Machineid
        	mainserverLocks[lockId].ownerMailboxNo = clientMailboxNo;	//setting the lock ownership i.e MailBoxNumber
        
//changes i made on subeh 28th oct
/*
		//Preparing to send the message.

printf("\nResponse104\n");
        	sprintf(ack, "%d", 1);						//preparing ack msg.
        	outMailHdr.length = strlen(ack) + 1;				
       		success = postOffice->Send(outPktHdr, outMailHdr, ack); 	//sending the message

		//Message Sent.

        	printf("Response sentn\n");
        	if ( !success ) 
        	{
            		printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
		    	mainserverLock->Release();
            		interrupt->Halt();
        	}
*/
//changes end here
    	}
    

	//if Lock not available then..
	
	else
    		{
    

		
   		
        	sprintf(ack, "%d", 1);				//preparing ack msg.
    		
		

		//Assigning a new wait queue for each lock.

		DEBUG('x',"\nLOCK IS NOT AVAI\n");
		for(int ctr2=0;ctr2<200;ctr2++)
		{
			mainserverLocks[lockId].waitingQueueLock[ctr2] = new waitQueue;
		}
	


         	//Assigning the  id and mailbox numberof the client to that particular lock

        	mainserverLocks[lockId].waitingQueueLock[mainserverLocks[lockId].waitingCount]->machineId = clientMachineId;
        	mainserverLocks[lockId].waitingQueueLock[mainserverLocks[lockId].waitingCount]->mailboxNo = clientMailboxNo;





		//Putting that request lock into the wait Queue of that particular lock.
 
        	mainserverLocks[lockId].LockwaitingQueue->Append((void *)mainserverLocks[lockId].waitingQueueLock[mainserverLocks[lockId].waitingCount]);




		//Incrementing the counter of the lock as the request has increased.

		mainserverLocks[lockId].waitingCount++;
   	}
    
	
	mainserverLock->Release();
    
}



char *Release_RPC(int lockId, int clientMachineId, int clientMailboxNo)
{
    

    		PacketHeader outPktHdr;				// Packet header object for network side
    		MailHeader outMailHdr;				// Mail header object for network side

    
   		char *ack = "Got it!";				//Ack msg that will be sent.
   		bool success; 					//using this to check if the message is sent correctly over the 								//network.
    

    		int MAX_PARAMETER = 10;        		// There should be maximum 10 comma seperated parameters in the request 							//message.
    

    		char *request[MAX_PARAMETER];		//character array that will contain the request message.


    		Lock* mainserverLock=new Lock("mainserverLock");	//Server Lock 


   		mainserverLock->Acquire();				//Acquring the server Lock



    
	//Checking if the lock is valid that we are releasing.


   	 if(lockId<0 || lockId > lcounter)
   	 {
    			printf("\nThe lock is not valid \n");
        		sprintf(ack, "%d", -1);
   	 }

    	else
    	{


	//Now that the lock is valid chekcing the ownership of the lock. 

        	if(mainserverLocks[lockId].ownerMachineId != clientMachineId || mainserverLocks[lockId].ownerMailboxNo != clientMailboxNo)
       		 {
    		
    			//if not the owner the we have to print an error message.

			printf("\nThe lock is not owned by the client\n");
            		sprintf(ack, "%d", -1);
        	}
        else
        {
    	
		//Checking the wait queue if someone is waiting 
            
		mainserverLocks[lockId].waitingQueueLock[mainserverLocks[lockId].waitingCount1] = (waitQueue *)mainserverLocks[lockId].LockwaitingQueue->Remove();
            
	
		//if no one waiting in the queue then set the lock state to be "AVAILABLE" and its machine id to be -1 and mailbox 			//id to be -1
		
		printf("The lock that is being released is= %s\n",mainserverLocks[lockId].name);

		if (mainserverLocks[lockId].waitingQueueLock[mainserverLocks[lockId].waitingCount1] == NULL)
            	{
    			
                	mainserverLocks[lockId].status = 1;			//lock is made available			
           	  	mainserverLocks[lockId].ownerMachineId = -1;		//machine id is set to -1
                	mainserverLocks[lockId].ownerMailboxNo = -1;		//mailbix number is set to -1.
                	sprintf(ack, "%d", 1);

			printf("The lock is released\n");

			
            	}
            
		else
            	{
			
		   //now that someone is waiting is waiting in the queue we 
		   //1.Get the first wait queue entry 
		   //2.Send a reply msg then.
	
                   	
                mainserverLocks[lockId].status = 0;		//Lock status is set to BUSY.
                
		//Copying the first entry in the wait queue to the reply msg.

		strcpy(ack, mainserverLocks[lockId].waitingQueueLock[mainserverLocks[lockId].waitingCount1]->replyMsg);
    		 

		//Setting a machine id to that lock.

		mainserverLocks[lockId].ownerMachineId = mainserverLocks[lockId].waitingQueueLock[mainserverLocks[lockId].waitingCount1]->machineId;

		//Setting a mailbox 
                mainserverLocks[lockId].ownerMailboxNo = mainserverLocks[lockId].waitingQueueLock[mainserverLocks[lockId].waitingCount1]->mailboxNo;



                mainserverLocks[lockId].waitingCount1++;		//increment the usage of the lock by incrementing the 										//waiting count

		//Sending the reply message

                outPktHdr.to = mainserverLocks[lockId].ownerMachineId;		
                outMailHdr.to = mainserverLocks[lockId].ownerMailboxNo;
                outMailHdr.length = strlen(ack) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, ack); 
                printf("Response sent\n");
                

		//Checking if the response is sent properly or not.


		if ( !success ) 
                {
                    printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
            		    mainserverLock->Release();
                    interrupt->Halt();
                }

                
    		}
        }
    }
  
    mainserverLock->Release();
    return ack;

}


















char * appendTS(char *TS, char *message, int length)
{
	char tempArr[MaxMailSize];
	bzero(tempArr,MaxMailSize);

	strncpy(tempArr, message, length);
	

	bzero(message,MaxMailSize);
	strncpy(message,TS,9);
	message[9]='@';
	
	strcat(message,tempArr);
	return message;	
}



#endif

void StartSimulation_Syscall()
{
	#ifdef NETWORK
	
	char buffer[MaxMailSize];
	
	PacketHeader outPktHdr, inPktHdr;		
    MailHeader outMailHdr, inMailHdr;		

	postOffice -> Receive(currentThread -> mailboxNo, &inPktHdr, &inMailHdr, buffer);
    
    fflush(stdout);
    
#endif

}


#ifdef NETWORK

void network_thread_function(int a)
{

	

	    	
    	
    	char *ack = "Got it!";
    	

    	//bool success; 						//using this to check if the message is sent correctly over the 								//network.
    	
	

    	char *param[100];			//character array that will contain the request message.
	int CVId;		
    	
    	char* requestmessage;
    	int i;
	int itemp1;
    	char name;
    	int lockId, CondvarId;
    	int found = 0;
    	Lock* mainserverLock=new Lock("mainserverLock");
    	int clientMachineId, clientMailboxNo;
	int myflag;
	myflag = 0;

	struct db *network_db;
	struct messageStruct *messageQueue;
	struct messageStruct *messageToBeProcessed;
	
	char *tempVar;
	char bufferTemp[MaxMailSize];
	
	long int timeStampExtracted;
	int64_t timeStampLong;
	
	long int timeStampConverted=0;
	
	List *messageQ;
	messageQ=new List;
	
	bool flag;
	
	messageToBeProcessed=NULL;
	
//	printf("I entered man\n");
	//Stuff for network thread goes here
	int tempCounter, noOfReceives;

	int MAX_PARAMETER = 10;         // There should be maximum 10 comma seperated parameters in the request message.

	int reqCtr;
	char *request[MAX_PARAMETER];			//character array that will contain the request message.		
	int type;
	
	int noOfUserThreads;

	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	char *data = "";
	
	char buffer[MaxMailSize];

	int myMailboxNo=currentThread -> mailboxNo;
	
	char *requestedMessage;

//	printf("reached first place\n");

	// Message Format -> Seq No | Type of Message | Timestamp | Request Type | Request's Info

	sequenceNoGeneratorLock -> Acquire();
	int sequenceNo=sequenceNoGenerator++;	// For the sequence no. in the msg
	sequenceNoGeneratorLock -> Release();

	int typeOfMsg=2;			// Type of message = 2 for the Group Registration Message	
						// from the network_thread side. NA THOUGH !!!		
	int requestType=-1;			// -1 as NA here
	time_t ltime;				// For the timestamp

	//	printf("reached second place\n");

	// Message Preperation ends here	


	//Group Registration Phase Starts *************************************

	// construct packet, mail header for original message
    	// To: destination machine, mailbox 0
    	// From: our machine, reply to: mailbox 1

//	sprintf(data, "%d@%d@%s@%d", sequenceNo, typeOfMsg, asctime(localtime(&ltime)), requestType);
	sprintf(data, "%d@%d@%d", sequenceNo, typeOfMsg, requestType);

	
    	outPktHdr.to = 0;					//Group Registration Server's Machine ID goes here		
    	outMailHdr.to = 0;
    	outMailHdr.from = myMailboxNo;
    	outMailHdr.length = strlen(data) + 1;

//		printf("Reached here\n");
    	// Send the first message
    	bool success = postOffice->Send(outPktHdr, outMailHdr, data); 
	//	printf("Reached here man !\n");
    	if ( !success ) {
      	printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      	interrupt->Halt();
    	}


	// Receive the number of UserThreads

	postOffice->Receive(myMailboxNo, &inPktHdr, &inMailHdr, buffer);
	requestedMessage=strtok(buffer, "@");

//	printf("mbox no.= %d requested message -> %d\n", myMailboxNo, int(requestedMessage));
	
	reqCtr=0;
	
	while (requestedMessage!= NULL) 
    	{
				request[reqCtr] = requestedMessage;
//		printf("requested message -> %d\n", atoi(request[reqCtr]));
		
      		requestedMessage = strtok(NULL,"@");		
	/*	if (tempCounter == 3)		//bringin down from 4 to 3
				{
	*/ 	
		/*			ASSERT(int(requestedMessage) == 0);
				}
*/		
    	}	

	noOfUserThreads=atoi(request[reqCtr]);

	
	network_db=new struct db[noOfUserThreads];
	
	
//	printf("user thread=%d\n", noOfUserThreads);

	noOfReceives=noOfUserThreads;
	
	
	//Go on receive for a certain number of times based on the no. of user threads
	
	int dbCounter=0;
	
	while(noOfReceives)
	{
		char newBuffer[MaxMailSize];
		
		postOffice->Receive(myMailboxNo, &inPktHdr, &inMailHdr, newBuffer);
		
        	// Creating Database
			reqCtr=0;

	//		printf("Received String -> %s\n", newBuffer);
			
        	requestedMessage = strtok(newBuffer,"@");
			request[reqCtr] = requestedMessage;
			
//			printf("Received -> %d\n", atoi(request[reqCtr]));
			
			requestedMessage = strtok(NULL,"@");			
			request[reqCtr] = requestedMessage;
			
//			printf("Received -> %d\n", atoi(request[reqCtr]));
			
			
			requestedMessage = strtok(NULL,"@");			
			request[reqCtr] = requestedMessage;
			network_db[dbCounter].machineID=atoi(request[reqCtr]);
			
	//		printf("MID-> %d\n", network_db[dbCounter].machineID);
	
			requestedMessage = strtok(NULL,"@");			
			request[reqCtr] = requestedMessage;
			network_db[dbCounter].mailboxNO=atoi(request[reqCtr]);

//			printf("MNo-> %d\n", network_db[dbCounter].mailboxNo);
			
			dbCounter++;
			noOfReceives--;
	}
	
/*	
	noOfReceives=noOfUserThreads / no_Per_Thread;

	if (noOfUserThreads % no_Per_Thread)
		noOfReceives++;


	// Must wait noOfReceives times

	int dbCounter=0;

	printf("noreceives -> %d\n", noOfReceives);
	
	while(noOfReceives)
	{
	
		reqCtr=0;
	
	char buffer[MaxMailSize];
		
		postOffice->Receive(myMailboxNo, &inPktHdr, &inMailHdr, buffer);
		
        	// Creating Database
		reqCtr=0;

			
        	requestedMessage = strtok(buffer,"@");
			request[reqCtr] = requestedMessage;
			printf("seq no.-> %d \n", atoi(request[reqCtr]));
			
//			printf("before second\n");
    		requestedMessage = strtok(NULL,"@");			
	//		printf("before third\n");
			request[reqCtr] = requestedMessage;
			printf("requested msg-> %d \n", atoi(request[reqCtr]));
			
//		ASSERT(int(requestedMessage) == 1);

//		printf("assert rocks now !!\n");
		
    		while (requestedMessage!= NULL) 
    		{
	//			printf("Entered while\n");
			
    			requestedMessage = strtok(NULL,"@");
				request[reqCtr] = requestedMessage;
				if (atoi(request[reqCtr]) == 1000)
				break;
		
//			printf("Entering data\n");
			
			network_db[dbCounter].machineID=atoi(request[reqCtr]);
			requestedMessage = strtok(NULL,"@");
				request[reqCtr] = requestedMessage;
			network_db[dbCounter].mailboxNo=atoi(request[reqCtr]);
			
			printf("I got-> %d %d\n", network_db[dbCounter].machineID, network_db[dbCounter].mailboxNo );
			
			
			dbCounter++;
    		}
			printf("wow reached here %d noofreceives %d mbox\n", noOfReceives, myMailboxNo);
			
			noOfReceives--;
	}
*/

	printf("\n\n\nMy mailbox no. is %d\n\n", myMailboxNo);
	printf("gonna print now\n");
	for(int abc=0; abc < noOfUserThreads; abc++)
		printf("Machine ID: %d\tMailbox No.: %d\n", network_db[abc].machineID, network_db[abc].mailboxNO);

	
	// The networking thread has the database by now

	// Figure out a way to delay the further steps till all the other networking threads have their 
	// database creation phase done

	//INSERT MAJOR DELAY HERE !!!!!!!!!!!!!!!!!!!!!

	//START SIMULATION CODE DO LATER !!!!
	
	//Now listen to messages. Messages can come from either a network thread or network thread's own user thread
		
	printf("Network Thread actually starts listening from now\n");
	
	int senderMailboxNO, senderMachineID;
	int receivedMsgLength;
	
	struct timeval timeStampValue;
	char timeStore[17];
	
	while(1)
	{
		bzero(buffer, MaxMailSize);
		
		postOffice->Receive(myMailboxNo, &inPktHdr, &inMailHdr, buffer);

		senderMachineID=inPktHdr.from;
		senderMailboxNO=inMailHdr.from;
		receivedMsgLength=inMailHdr.length;

		//Case 1: The Message is from a User Thread
		//network thread's mailbox NO is user thread's mailbox Number+1
		 
		if( (senderMailboxNO == (currentThread -> mailboxNo - 1)) && (senderMachineID == postOffice -> GetMachineID() ) )
		{
			printf("Got a message from a user thread \n");
		
			gettimeofday(&timeStampValue, NULL);
			sprintf(timeStore,"%11d%6d", timeStampValue.tv_sec, timeStampValue.tv_usec);
		
			for(int m=0; m < 17; m++)
				if(timeStore[m]==' ')
					timeStore[m]='0';
		
//			printf("reached here\n");
		
			appendTS(timeStore + 6, buffer, receivedMsgLength);
		
		
			for(int m=0; m < noOfUserThreads; m++)
			{
				outPktHdr.to = network_db[m].machineID;					
			    outMailHdr.to = network_db[m].mailboxNO;
				
	/*			clientMID= states[i].mid;					
			    clientMBN=states[i].mbn;
	*/			
		    	outMailHdr.from = currentThread -> mailboxNo;
				outMailHdr.length = strlen(buffer) + 1;		
				success = postOffice -> Send(outPktHdr, outMailHdr, buffer); 	
				
					if ( !success ) 		
					{
						printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
						interrupt->Halt();
					}				
			}		

			
		}

		
		//Case 2 - Another network thread must have sent the packet
		else
		{
			int tempValue;
		
			tempVar=NULL;
			bzero(bufferTemp, MaxMailSize);
			
			strcpy(bufferTemp, buffer);
				
			//Extract the timestamp
			tempVar=strtok(buffer,"@");
			
			//Extracted the timestamp now
			timeStampExtracted=atol(tempVar);
			
			tempVar=strtok(NULL,"@");
			tempValue=atoi(tempVar);

			
			//sent from other network thread
			
			if(tempValue != 10)					//10 was timestamp value
			{
				//Create a TimeStamp
				gettimeofday(&timeStampValue, NULL);
				sprintf(timeStore,"%11d%6d", timeStampValue.tv_sec, timeStampValue.tv_usec);
		
				for(int m=0; m < 17; m++)
					if(timeStore[m]==' ')
						timeStore[m]='0';

			
				messageQueue=new messageStruct;

				messageQueue -> timeStamp=timeStampExtracted;
				bzero(messageQueue -> message, MaxMailSize);
				strcpy(messageQueue -> message, bufferTemp);
				
				messageQueue -> machineID=inPktHdr.from;
				messageQueue -> mailboxNO=inMailHdr.from;

	
				//Updating TimeStamp
				for(int m=0; m < noOfUserThreads; m++)
					if(network_db[m].mailboxNO == inMailHdr.from && network_db[m].machineID == inPktHdr.from)
						if(network_db[m].timeStamp < timeStampExtracted)
							network_db[m].timeStamp=timeStampExtracted;
				
				
				timeStampLong=100 * inPktHdr.from + inMailHdr.from + 1000 * timeStampExtracted;
				
				//Sort Message Queue by timestamp
				messageQ -> SortedInsert((void *)messageQueue, timeStampExtracted);
		
				
				bzero(buffer, MaxMailSize);
			
				strcpy(buffer, "10@");
				
				appendTS(timeStore + 6, buffer, 3);
		
		
				for(int m=0; m < noOfUserThreads; m++)
				{
					outPktHdr.to = network_db[m].machineID;					
					outMailHdr.to = network_db[m].mailboxNO;
				
		/*			clientMID= states[i].mid;					
					clientMBN=states[i].mbn;
		*/			
					outMailHdr.from = currentThread -> mailboxNo;
					outMailHdr.length = strlen(buffer)+1;		
					success = postOffice -> Send(outPktHdr, outMailHdr, buffer); 	
				
					if ( !success ) 		
					{
						printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
						interrupt->Halt();
					}				
				}
		
			}
		
		
			
				//Updating TimeStamp
			for(int m=0; m < noOfUserThreads; m++)
				if(network_db[m].mailboxNO == inMailHdr.from && network_db[m].machineID == inPktHdr.from)
					if(network_db[m].timeStamp < timeStampExtracted)
						network_db[m].timeStamp=timeStampExtracted;


			while(true)
			{
				messageToBeProcessed=(struct messageStruct *) messageQ -> SortedRemove(&timeStampLong);
			
				flag=true;
			
				timeStampConverted=timeStampLong / 100;
			
				if(messageToBeProcessed)
				{
					for(int m=0; m < noOfUserThreads; m++)
						if(network_db[m].timeStamp < timeStampConverted)
						{
							messageQ -> SortedInsert((void *)messageToBeProcessed, timeStampLong);				
				
							flag=false;
						
							break;
						}
				}
				else
					break;
			
				if(flag)
				{	






	//AFTER THIS
					tempVar=strtok(messageToBeProcessed -> message, "@");
		
					//Contains Timestamp now
		
					tempVar= strtok(NULL, "@");
					itemp1=0;
	
					while (tempVar != NULL) 
					{
						param[itemp1] = tempVar;
						itemp1++;
						tempVar= strtok(NULL, "$");
					}
						
					requestType = atoi(param[0]);
										
					tempVar=NULL;

	 //SENDING THE HEADERS OF THE REPLY MESSAGE.

        outPktHdr.to = messageToBeProcessed -> machineID;
        outMailHdr.to = messageToBeProcessed -> mailboxNO;
						
					switch (requestType) 
						{
			
default:
    		printf("THE REQUEST IS UNKNOWN.\n");
		interrupt->Halt();
		break;

       	    case 1 :
        		printf("CreateLock Request.\n");
     
                	mainserverLock->Acquire();


       		//CHECKING IF THE LOCK ALREADY EXISTS.

        	int ctr2;
        	if(lcounter>0)
        	{
                    for(ctr2=0; ctr2<lcounter; ctr2++)
                    {
                        if( strcmp(mainserverLocks[ctr2].name, param[2]) == 0 )
                        {   
                            found = 1;
                            DEBUG('x',"%s already exists and the lock is %d\n",param[2],ctr2);
							
                            break;
                        }
                        
                    }
                }

		//If the number of locks that are created are greater than the total number of locks available.

                else if(lcounter>=MAX_LOCKS)
                {
                    DEBUG('x',"Cannot create lock\n");
                    sprintf(ack, "%d", -1);
                }

        		

		//If the lock was to be found then return the lock.
                if(found == 1)
                {
                    lockId=mainserverLocks[ctr2].myId;
                    sprintf(ack, "%d", lockId);
            		found = 0;
                }

		//If the lock was not found then create a new lock and return the lock number.

                else
                {
			
		    //setting the attributes of the lock
                    strcpy(mainserverLocks[lcounter].name, param[2]);			
                    mainserverLocks[lcounter].myId = lcounter;
                    mainserverLocks[lcounter].status = 1;
                    mainserverLocks[lcounter].ownerMachineId = -1;
                    mainserverLocks[lcounter].ownerMailboxNo = -1;
                    mainserverLocks[lcounter].waitingCount = 0;
                    mainserverLocks[lcounter].waitingCount1 = 0;

		    //initializing the waitqueue of all the locks.
                    for(i=0; i<200; i++)
			{
                        	mainserverLocks[lcounter].waitingQueueLock[i] = new waitQueue;
                    	}
			
			mainserverLocks[lcounter].LockwaitingQueue = new List;
                    	sprintf(ack, "%d", lcounter);
					
                    lcounter++;
                }
                mainserverLock->Release();
                
		//Sending the reply message.

                outMailHdr.length = strlen(ack) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, ack); 
                printf("Response sent\n");

		//checking if it is delieverd or not.


                if ( !success ) 
                {
                    printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
           		    mainserverLock->Release();
                    interrupt->Halt();
                }
    		break;
    		
    	    case 2:
	
        		printf("DestroyLock Request.\n");
        		
                	mainserverLock->Acquire();
                	lockId = atoi(param[1]);

               //Checking if the lock id is greater than 0 and greater than the number of locks assigned till now.


                if (lockId < 0 || lockId > lcounter)
                {
                	printf("Invalid lock passed to destroy\n");
					sprintf(ack, "%d", -1);
                }		



		//We check weather someone is waiting for the lock.


                else if ((mainserverLocks[lockId].waitingCount - mainserverLocks[lockId].waitingCount1) > 0)
                {
                	printf("cannot destroy lock because someone is waiting for it.\n");
					sprintf(ack, "%d", -1);
                }

	

		//if not then we destroy the lock by:
		//1.setting its id to -1
		//2.setting its status to busy.
		//3.Inavlidating its machine id.
		//4.Invaldiating its mailbox number.


                else

                {
                    //Destroying lock..

		    printf("The lock that has been destroyed has an id = %d",lockId);
                    strcpy(mainserverLocks[lockId].name, "");
                    mainserverLocks[lockId].myId = -1;
                    mainserverLocks[lockId].status = 0;
                    mainserverLocks[lockId].ownerMachineId = -1;
                    mainserverLocks[lockId].ownerMailboxNo = -1;
                    mainserverLock->Release();
                    sprintf(ack, "%d", 1);
                }
                  

		//Sending reply message that the task has been done properly.
 
                outMailHdr.length = strlen(ack) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, ack); 
                printf("Response sent\n");


		//Checking wheather the message that was sent was recieved or not..

                if ( !success ) 
                {
                    printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
           	    mainserverLock->Release();
                    interrupt->Halt();
                }
           
    		break;
    		
    	    case 3:

	
        	printf("Acquire Request.\n");
    		
    		    mainserverLock->Acquire();
    		
                lockId = atoi(param[1]);			//getting the lock number.
    		
		

		//Getting the machine id number and the mailbox number.

                clientMachineId = messageToBeProcessed -> machineID;
                clientMailboxNo = messageToBeProcessed -> mailboxNO;
                
    		//Checking if the lock that it is going to acquire is valid or not.


    		    if(lockId<0 || lockId > lcounter)
    		    {

				//if the lock is valid then send the message that it cannot acquire the lock.

				printf("\n The lock is invalid cannot acquire the lock!!!!!!!\n\n");
                    		sprintf(ack, "%d", -1);
                    		outMailHdr.length = strlen(ack) + 1;
                    		success = postOffice->Send(outPktHdr, outMailHdr, ack); 
                    		
				//checking if the emssage was successfully delieverd or not.


                    		if ( !success ) 
                   		 {
                      			  printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
              		    		  mainserverLock->Release();
                        		  interrupt->Halt();
                    		 }
    		    }
    		    else

		    // The lock that is requested to acquire is valid.
    		    {
            
                	mainserverLock->Release();
            
                    	Acquire_RPC(lockId, clientMachineId, clientMailboxNo);
			//Preparing to send the message.


        		sprintf(ack, "%d", 1);						//preparing ack msg.
        		outMailHdr.length = strlen(ack) + 1;				
       			success = postOffice->Send(outPktHdr, outMailHdr, ack); 	//sending the message

			//Message Sent.

        		printf("Response sent\n");
        		if ( !success ) 
        		{
            			printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
		    		mainserverLock->Release();
            			interrupt->Halt();
        		}
            
                    }

    		
    		break;
   		
    	    case 4:

    		printf("Release Request.\n");
    		
    		mainserverLock->Acquire();
                lockId = atoi(param[1]);				//getting the lock number
		
		//getting the  client Machine Id and Mailbox No.


                clientMachineId = messageToBeProcessed -> machineID;
                clientMailboxNo = messageToBeProcessed -> mailboxNO;
                
            	//releasing the lock
			
                    ack = Release_RPC(lockId, clientMachineId, clientMailboxNo);
           
                    
		    //sending the reply message.

		    outMailHdr.length = strlen(ack) + 1;
                    success = postOffice->Send(outPktHdr, outMailHdr, ack); 
                    printf("Response sent\n");

		    //Checking if the reply message was deleiverd or not.
                    if ( !success ) 
                    {
                        printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
              		mainserverLock->Release();
                        interrupt->Halt();
                    }

    		break;
    		
    	    
			
			
			
		case 5:
	
    		printf("CREATING A CONDITION .\n");
    		
    			int flag1;		//TO CHECK WHEATHER THE CONDITION EXISTS OR NOT BEFORE HAND.
			int cnt;
			int myCVId;
			mainserverLock->Acquire();
			flag1 = 0;
			myCVId = 0;

			//CHECKING IF THE CONDITION VARIABLE HAS BEEN CREATED BEFORE OR NOT.

			if (ConditionCounter > 0)
			{
				for (cnt=0;cnt<ConditionCounter;cnt++)
				{
					if ( strcmp(mainserverCVs[cnt].name, param[2]) == 0)
					{
						sprintf(ack, "%d", mainserverCVs[cnt].CId);
						flag=1;
						 DEBUG('x', "%d\n",flag1);
						DEBUG('x',"%d has the same CV name as %s = %s \n",cnt,mainserverCVs[cnt].name,request[2]);		
						break;
					}
				}
			}


    		

				//FLAG IS SET TO 1 IF THE CONDITION EXISTS BEFORE HAND.


				if (flag1==1)
				{		DEBUG('x',"\nTHIS CONDITION EXISTS BEFORE\n");
						myCVId = mainserverCVs[cnt].CId;					
						sprintf(ack, "%d",myCVId);
        					flag1 = 0;

				}
				else
				{
            
			//ADDING A NEW CV TO THE LIST.
			
					strcpy(mainserverCVs[ConditionCounter].name,request[2]);
					mainserverCVs[ConditionCounter].CId = ConditionCounter;
					mainserverCVs[ConditionCounter].myLockID = -1;
					mainserverCVs[ConditionCounter].valid = 1;
					mainserverCVs[ConditionCounter].waitingCount = 0 ;
					mainserverCVs[ConditionCounter].waitingArrayCtr = 0;
					mainserverCVs[ConditionCounter].waitingArrayCtr1 = 0;
					mainserverCVs[ConditionCounter].WaitList = new List;

					sprintf(ack, "%d", mainserverCVs[ConditionCounter].CId);

				    printf("Creating new CV with name = %s\n Condition Variable id = %d\n Lock Id = %d\n",mainserverCVs[ConditionCounter].name,mainserverCVs[ConditionCounter].CId,mainserverCVs[ConditionCounter].myLockID);

			//Incrementing the condition counter.
				    ConditionCounter++;
				}

                mainserverLock->Release();

		
		//Reply message is sent.

                outMailHdr.length = strlen(ack) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, ack); 

		
                printf("Response sent\n");
                
		//If response is not sent then verify.
		
		if ( !success ) 
                {
                    printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
            	    mainserverLock->Release();
                    interrupt->Halt();
                }
    		break;

			



    	    case 6:


        		printf("DestroyCondition Request.\n");

       		    	mainserverLock->Acquire();
                
			CVId = atoi(param[1]);

                //Checking if the condition that is being destroyed is valid or not.


                if (CVId < 0 || CVId > ConditionCounter)
                {
                	printf("invalid condition\n Returning\n");
			sprintf(ack, "%d", -2);
                }



                //Checking if the condtiton that is being deleted exists or not.

                else if (mainserverCVs[CVId].valid != 1)
                {
                	printf("invalid condition bec already deleted\n Returning\n");
					sprintf(ack, "%d", -2);
                }




		//If someone is waiting on the condtition then we cannot sestroy the condition.

                else if (mainserverCVs[ConditionCounter].waitingCount != 0)
                {
                	printf("invalid condition bec someone is waiting\n Returning\n");
					sprintf(ack, "%d", -2);
                }



                // As now the condition exists and is valid so deleting the condition.


                else 
            	{
            		printf("Condition %d with name %s is deleted\n",mainserverCVs[CVId].CId,mainserverCVs[CVId].name);
            		strcpy(mainserverCVs[CVId].name,"");
            		DEBUG('x',"%s\n",mainserverCVs[CVId].name);
            		mainserverCVs[CVId].valid=-1;	
            		mainserverCVs[CVId].CId = -1;
            		mainserverCVs[CVId].myLockID = -1;
            		mainserverCVs[CVId].waitingCount = 0 ;
			sprintf(ack, "%d", 1);
            	}



		//preparing to send the response.


                	outMailHdr.length = strlen(ack) + 1;
                	success = postOffice->Send(outPktHdr, outMailHdr, ack); 
               	 	printf("Response sent\n");

                	if ( !success ) 
                	{
                    	printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
            		    mainserverLock->Release();
                    	interrupt->Halt();
                	}

    		break;




    	    case 7:


        		 DEBUG('x', "Wait Request.\n");
   
    			CVId = 0;
    			lockId = 0;
    		
    			CVId =atoi(param[2]);			//geting a condition variable number.
        		
    			lockId = atoi(param[1]);			//getting a lock number.
    		
	

			//Displaying the machine id and the mailbox number.


    			printf("Wait request for CVId = %d\n Lockid = %d\n from MachineId = %d\n MailboxNo = %d\n",CVId,lockId,clientMachineId,clientMailboxNo);



                // Chekcing if the condition variable exists or not.


//MY CHANGES
		//Acquire_RPC(lockId,clientMachineId,clientMailboxNo);

//MY CHANGES FINISHED

                if (CVId<0 || CVId> MAX_CONDITIONS)
                {
                	printf("invalid condition\n Returning\n");
    				sprintf(ack, "%d", -1);
                }


		//Checking if the condition has already been deleted or not?

                else if (mainserverCVs[CVId].valid != 1)
                {
                	printf("invalid condition bec already deleted\n Returning\n");
    				sprintf(ack, "%d", -1);
                }


                //checking if the lock id for the condition exists or not or the lock counter is 0 or lockid passed is greater than 			//the lock counter



                else if (mainserverLocks[lockId].myId == -1 || lcounter == 0 || lockId > lcounter)
                {
                	printf("Lock does not exist\n Returning\n");
    				sprintf(ack, "%d", -1);
                }


                // also checking  if the condition owns the lock or not.


                else if (mainserverLocks[lockId].ownerMachineId != clientMachineId || mainserverLocks[lockId].ownerMailboxNo != clientMailboxNo)
                {
                	 printf("\nyour machine id %d != %d or mailBoxNumber %d != %d\n Returning\n",mainserverLocks[lockId].ownerMachineId,clientMachineId,mainserverLocks[lockId].ownerMailboxNo,clientMailboxNo);
    				sprintf(ack, "%d", -1);
                }

	else { 


		//else we put it in the wait queue.

		mainserverCVs[CVId].waitingCount++;


		// setting its attributes


		mainserverCVs[CVId].myLockID = lockId;
		mainserverCVs[CVId].locksmacID = clientMachineId;
		mainserverCVs[CVId].LockMailboxNo = clientMailboxNo;
		mainserverCVs[CVId].waitingThread[mainserverCVs[CVId].waitingArrayCtr1].machineId = clientMachineId;
		mainserverCVs[CVId].waitingThread[mainserverCVs[CVId].waitingArrayCtr1].mailboxNo = clientMailboxNo;

//change
		mainserverCVs[CVId].waitingArrayCtr1++;
//change

		//release the lock that is held by the condition


 		ack = Release_RPC(lockId,clientMachineId,clientMailboxNo);
	break;
		}

		   // sending a reply message

                    outMailHdr.length = strlen(ack) + 1;
                    success = postOffice->Send(outPktHdr, outMailHdr, ack); 
                    printf("Response sent\n");

		   //validating it.


                    if ( !success ) 
                    {
                        printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
              		    mainserverLock->Release();
                        interrupt->Halt();
                    }

    		break;
    	
			





    	    case 8:



    			 DEBUG('x', "Signal Request.\n");
  
    			CVId = 0;
    			lockId = 0;
    			
    			CVId =atoi(param[2]);			//getting the condition variable number
        		
    			lockId = atoi(param[1]);		//getting the lock number.
    			


			//displaying the machine id and the mailbox number.

    			printf("Signal request for CVId = %d\n Lockid = %d\n from MachineId = %d\n MailboxNo = %d\n",CVId,lockId,clientMachineId,clientMailboxNo);



                  // Chekcing if the condition variable exists or not.


Acquire_RPC(lockId,mainserverCVs[CVId].waitingThread[mainserverCVs[CVId].waitingArrayCtr].machineId , mainserverCVs[CVId].waitingThread[mainserverCVs[CVId].waitingArrayCtr].mailboxNo);

                if (CVId<0 || CVId> MAX_CONDITIONS)
                {
                	DEBUG('x',"invalid condition\n Returning\n");
    				sprintf(ack, "%d", -1);
                }

		

		//Checking if the condition has already been deleted or not?

                else if (mainserverCVs[CVId].valid != 1)
                {
                	DEBUG('x',"invalid condition bec already deleted\n Returning\n");
    				sprintf(ack, "%d", -1);
                }
                


		//checking if the lock id for the condition exists or not or the lock counter is 0 or lockid passed is greater than 			//the lock counter


                else if (mainserverLocks[lockId].myId == -1 || lcounter == 0 || lockId > lcounter)
                {
                	DEBUG('x',"Lock does not exist\n Returning\n");
    				sprintf(ack, "%d", -1);
                }




                 // also checking  if the condition owns the lock or not.



                else if (mainserverLocks[lockId].ownerMachineId != clientMachineId || mainserverLocks[lockId].ownerMailboxNo != clientMailboxNo)
                {
                	 DEBUG('x', "your machine id %d != %d or mailBoxNumber %d != %d\n Returning\n",mainserverLocks[lockId].ownerMachineId,clientMachineId,mainserverLocks[lockId].ownerMailboxNo,clientMailboxNo);
    				sprintf(ack, "%d", -1);
                }
	

		//if wait is on a different lock..


		else if (mainserverCVs[CVId].myLockID != lockId)
		{
			DEBUG('x',"Wait is on different Lock .......\nwait is on a lock =%d and you have lock =%d\n",mainserverCVs[CVId].myLockID,lockId);
			sprintf(ack, "%d", -1);
		}



		//checking for waiting threads. 


		else if (mainserverCVs[CVId].waitingArrayCtr >= mainserverCVs[CVId].waitingArrayCtr1)
		{
					printf("There is no thread waiting on CVId = %d as %d >= %d\n",CVId,mainserverCVs[CVId].waitingArrayCtr,mainserverCVs[CVId].waitingArrayCtr1);
					sprintf(ack, "%d", -1);
		}
				
				
		else
		{


		//All the conditions have been fullfilled we can now signal.


		 printf("Now signalling machineId = %d and mailboxNo = %d",mainserverCVs[CVId].waitingThread[mainserverCVs[CVId].waitingArrayCtr].machineId,mainserverCVs[CVId].waitingThread[mainserverCVs[CVId].waitingArrayCtr].mailboxNo);	
		
		Acquire_RPC(lockId,mainserverCVs[CVId].waitingThread[mainserverCVs[CVId].waitingArrayCtr].machineId , mainserverCVs[CVId].waitingThread[mainserverCVs[CVId].waitingArrayCtr].mailboxNo);


					mainserverCVs[CVId].waitingArrayCtr++;
					sprintf(ack, "%d", 1);

				}
					
			

			//sending a reply message.

	
                    outMailHdr.length = strlen(ack) + 1;
                    success = postOffice->Send(outPktHdr, outMailHdr, ack); 
                    printf("Response sent\n");

			//validating if it was deleiverd or not.


                    if ( !success ) 
                    {
                        printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
              		    mainserverLock->Release();
                        interrupt->Halt();
                    }
	

    		break;
    		



    	    case 9:
    		 printf("Broadcast Request.\n");
    		
    			CVId = 0;
    			lockId = 0;
    			
    			CVId =atoi(param[2]);				//getting the conditon variable number.
        		
    			lockId = atoi(param[1]);				//getting the lock number.
    			


			//displaying the machine id and the machine mail box number.


    			printf("Broadcast request for CVId = %d\n Lockid = %d\n from MachineId = %d\n MailboxNo = %d\n",CVId,lockId,clientMachineId,clientMailboxNo);



               // Chekcing if the condition variable exists or not.


                if (CVId<0 || CVId> MAX_CONDITIONS)
                {
                	DEBUG('x',"Invalid condition\n Returning\n");
    				sprintf(ack, "%d", -1);
                }


		//Checking if the condition has already been deleted or not?


                else if (mainserverCVs[CVId].valid != 1)
                {
                	DEBUG('x',"Invalid condition bec already deleted\n Returning\n");
    				sprintf(ack, "%d", -1);
                }




		//checking if the lock id for the condition exists or not or the lock counter is 0 or lockid passed is greater than 			//the lock counter

               
                else if (mainserverLocks[lockId].myId == -1 || lcounter == 0 || lockId > lcounter)
                {
                	DEBUG('x',"Lock does not exist\n Returning\n");
    				sprintf(ack, "%d", -1);
                }


			//if wait is on a different lock..




		else if (mainserverCVs[CVId].myLockID != lockId)
				{
					DEBUG('x',"Wait is on different Lock .......\nwait is on a lock =%d and you have lock =%d\n",mainserverCVs[CVId].myLockID,lockId);
					sprintf(ack, "%d", -1);
				}


			//if there are no threads waiting



				else if (mainserverCVs[CVId].waitingArrayCtr >= mainserverCVs[CVId].waitingArrayCtr1)
				{
					DEBUG('x',"There is no thread waiting on CVId = %d as %d >= %d\n",CVId,mainserverCVs[CVId].waitingArrayCtr,mainserverCVs[CVId].waitingArrayCtr1);
					sprintf(ack, "%d", -1);
				}
				
				
				else{



				//all condition fLockmyIdullfilled so broadcasting now..


					while (mainserverCVs[CVId].waitingArrayCtr < mainserverCVs[CVId].waitingArrayCtr1)
					{
					

					 printf("Now signalling machineId = %d and mailboxNo = %d",mainserverCVs[CVId].waitingThread[mainserverCVs[CVId].waitingArrayCtr].machineId,mainserverCVs[CVId].waitingThread[mainserverCVs[CVId].waitingArrayCtr].mailboxNo);	


					//acquiring the particular lock for that particular condition.

					Acquire_RPC(lockId,mainserverCVs[CVId].waitingThread[mainserverCVs[CVId].waitingArrayCtr].machineId,mainserverCVs[CVId].waitingThread[mainserverCVs[CVId].waitingArrayCtr].mailboxNo);

				mainserverCVs[CVId].waitingArrayCtr++;

					}

					sprintf(ack, "%d", 1);

				}
					

		    //Sendig a reply message.

	
                    outMailHdr.length = strlen(ack) + 1;
                    success = postOffice->Send(outPktHdr, outMailHdr, ack); 
                    printf("Response sent\n");


		    //validating the data..


                    if ( !success ) 
                    {
                        printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
              		mainserverLock->Release();
                        interrupt->Halt();
                    }

    		break;


case 10:

		printf("Create monitor variable request.\n");
     
                	
	
		

       		//CHECKING IF THE LOCK ALREADY EXISTS.

        	int ctr7;
        	if(mvcounter>0)
        	{
                    for(ctr7=0; ctr7<=mvcounter; ctr7++)
                    {	
                        if( strcmp(MV[ctr7].name, param[2]) == 0 )
                        {   
                            found = 1;
                            DEBUG('x',"%s already exists and the MV is %d\n",param[2],ctr7);
							
                            break;
                        }
                        
                    }
                }

		//If the number of locks that are created are greater than the total number of locks available.

                else if(mvcounter>=MAX_MV)
                {
                    DEBUG('x',"Cannot create lock\n");
                    sprintf(ack, "%d", -1);
                }

        		

		//If the MV was to be found then return the lock.
                if(found == 1)
                {int mvId;
			printf("e");
                    mvId=MV[ctr7].myId;
                    sprintf(ack, "%d", MV[ctr7].myId);
            		found = 0;
                }

		//If the lock was not found then create a new lock and return the lock number.

                else
                {
				
		    //setting the attributes of the lock
                   
		    strcpy(MV[mvcounter].name, param[2]);
			
		   			
                    MV[mvcounter].myId = mvcounter;
	            MV[mvcounter].value=0;
		    printf("Monitor Variable with name %s is created",MV[mvcounter].name);					
   		    mvcounter++;
		    
			
		    sprintf(ack,"%d",1);
                }
                
               
		//Sending the reply message.

                outMailHdr.length = strlen(ack) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, ack); 
                printf("Response sent\n");

		//checking if it is delieverd or not.


                if ( !success ) 
                {
                    printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
           		    
                    interrupt->Halt();
                }
    		break;



             case 11:

printf("Get value of monitor variable request.\n");
     
                	


       		//CHECKING IF THE LOCK ALREADY EXISTS.

        	int ctr20;
		
        	if(mvcounter>0)
        	{
                    for(ctr20=0; ctr20<mvcounter; ctr20++)
                    {
                        if( strcmp(MV[ctr20].name, param[2]) == 0 )
                        {   
                            found = ctr20;
                            
                            break;
                        }
                        
                    }
                }
		
		if(found==0)
		{
			printf("The monitor variable does not exist\n");
		 	sprintf(ack,"%d",-1);
			outMailHdr.length = strlen(ack) + 1;
                	success = postOffice->Send(outPktHdr, outMailHdr, ack); 
                	printf("Response sent\n");
			break;
		}
			
			
		    //setting the attributes of the MV
                    			
                   int value1;
	           value1= MV[ctr20].value;					
   		   
		   
		    sprintf(ack,"%d",value1);
                
                
                
		//Sending the reply message.

                outMailHdr.length = strlen(ack) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, ack); 
                printf("Response sent\n");

		//checking if it is delieverd or not.


                if ( !success ) 
                {
                    printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
           		   
                    interrupt->Halt();
                }
    		break;


            
        case 12:


printf("Set monitor variable request.\n");
     
                	


       		//CHECKING IF THE LOCK ALREADY EXISTS.


        	int ctr8;
        	if(mvcounter>0)
        	{
                    for(ctr8=0; ctr8<mvcounter; ctr8++)
                    {
                        if( strcmp(MV[ctr8].name, param[2]) == 0 )
                        {   
                            found = ctr8;
                            
                            break;
                        }
                        
                    }
                }
		

		if(found==0)
		{
			printf("The monitor variable does not exist\n");
		 	sprintf(ack,"%d",-1);
			outMailHdr.length = strlen(ack) + 1;
                	success = postOffice->Send(outPktHdr, outMailHdr, ack); 
                	printf("Response sent\n");
			break;
		}
			
			
		    //setting the attributes of the MV
                    
	            MV[ctr8].value= atoi(param[3]);					
   		    
		    printf("Monitor Variable with name %s has its monitor variable incremented its value is = %d",MV[ctr8].name,MV[ctr8].value);
		    sprintf(ack,"%d",1);
                
                
                
		//Sending the reply message.

                outMailHdr.length = strlen(ack) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, ack); 
                printf("Response sent\n");

		//checking if it is delieverd or not.


                if ( !success ) 
                {
                    printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
           		    
                    interrupt->Halt();
                }
    		break;

    		 case 13:

printf("Increment the value of the monitor variable \n");
     
                	


       		//CHECKING IF THE LOCK ALREADY EXISTS.

        	int ctr11;
        	if(mvcounter>0)
        	{
                    for(ctr11=0; ctr11<mvcounter; ctr11++)
                    {
                        if( strcmp(MV[ctr11].name, param[2]) == 0 )
                        {   
                            found = ctr11;
                            
                            break;
                        }
                        
                    }
                 }
		
		if(found==0)
		{
			printf("The monitor variable does not exist\n");
		 	sprintf(ack,"%d",-1);
			outMailHdr.length = strlen(ack) + 1;
                	success = postOffice->Send(outPktHdr, outMailHdr, ack); 
                	printf("Response sent\n");
			break;
		}
			
		
			
		    //setting the attributes of the MV
                    			
                   
	           MV[ctr11].value++;					
   		    printf("The value of the monitor variable %s is now %d",MV[ctr11].name,MV[ctr11].value);
		   
		    sprintf(ack,"%d",1);
                
                
                
		//Sending the reply message.

                outMailHdr.length = strlen(ack) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, ack); 
                printf("Response sent\n");

		//checking if it is delieverd or not.


                if ( !success ) 
                {
                    printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
           		    
                    interrupt->Halt();
                }
    		break;



    		
    		
        case 14:

printf("Decrement the value of the monitor variable\n");
     
                	


       		//CHECKING IF THE  ALREADY EXISTS.

        	int ctr9;
        	if(mvcounter>0)
        	{
                    for(ctr9=0; ctr9<mvcounter; ctr9++)
                    {
                        if( strcmp(MV[ctr9].name, param[2]) == 0 )
                        {   
                            found = ctr9;
                            
                            break;
                        }
                        
                    }
                }
		

		if(found==0)
		{
			printf("The monitor variable does not exist\n");
		 	sprintf(ack,"%d",-1);
			outMailHdr.length = strlen(ack) + 1;
                	success = postOffice->Send(outPktHdr, outMailHdr, ack); 
                	printf("Response sent\n");
			break;
		}
			
			
		    //setting the attributes of the MV
                    			
                   
	           MV[ctr9].value--;					
   		   
		   printf("The value of the monitor variable %s is now %d",MV[ctr9].name,MV[ctr9].value);
		    sprintf(ack,"%d",1);
                
                
                
		//Sending the reply message.

                outMailHdr.length = strlen(ack) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, ack); 
                printf("Response sent\n");

		//checking if it is delieverd or not.


                if ( !success ) 
                {
                    printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
           		    
                    interrupt->Halt();
                }
    		break;

case 15:

printf("Delete monitor variable request.\n");
     
                	


       		//CHECKING IF THE LOCK ALREADY EXISTS.

        	int ctr10;
        	if(mvcounter>0)
        	{
                    for(ctr10=0; ctr10<mvcounter; ctr10++)
                    {
                        if( strcmp(MV[ctr10].name, param[2]) == 0 )
                        {   
                            found = ctr10;
                            DEBUG('x',"%s already exists and the MV is %d\n",param[2],ctr2);
							
                            break;
                        }
                        
                    }
                }

		
        	if(found==0)
		{
			printf("The monitor variable does not exist\n");
		 	sprintf(ack,"%d",-1);
			outMailHdr.length = strlen(ack) + 1;
                	success = postOffice->Send(outPktHdr, outMailHdr, ack); 
                	printf("Response sent\n");
			break;
		}
			

		
		    printf("Monitor Variable with name %s is deleted\n",MV[ctr10].name);
		    //setting the attributes of the lock
                    sprintf(MV[ctr10].name,"%s","");			
                    MV[ctr10].myId =-1;
	            MV[ctr10].value=-1;					
   		    mvcounter--;
		    sprintf(ack,"%d",1);
                
                
                
		//Sending the reply message.

                outMailHdr.length = strlen(ack) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, ack); 
                printf("Response sent\n");

		//checking if it is delieverd or not.


                if ( !success ) 
                {
                    printf("The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
           		    
                    interrupt->Halt();
                }
    		break;

    	}

 
    


}

	
			
			
			
			
	
}		
}		
}
}

#endif


void exec_thread_function(int a)
{

	//Initialize the register by using currentThread->space
	currentThread -> space -> InitRegisters();	

	
	//Call  Restore State  through currentThread->space
	currentThread -> space -> RestoreState();	


	//Call machine->Run() 
	machine->Run();
}


int Exec_Syscall(unsigned int vaddr, int len)
{
	
	printf("Entering Exec");

	char *buf = new char[len+1];	           // Kernel buffer for input
    	OpenFile *f;	                           // Open file for output
  

	//Convert the virtual address into the physical address to obtain the name of the process to be executed
                                            
  	int result = copyin(vaddr,len,buf);
	if (result == -1) 
	{
		DEBUG('x',"\nVirtual Address is not correct.\n");
		delete buf;
		return -1;
	}
 

	//Now Opening the file using filesystem->Open 
	//Store its openfile pointer

	f = fileSystem->Open(buf);
	if(f == NULL)
	{
    		DEBUG('x',"Cannot open Files\n");
    		return -1;
	}	

	delete[] buf;
	

	//Create new addrespace for this executable file
	pInfoLock -> Acquire();


	AddrSpace *addressSpaceForExecThread;

	addressSpaceForExecThread=new AddrSpace(f);
	addressSpaceForExecThread -> spaceID=processUniqueID;			//The process's unique ID is the addrSpace's uniqueID

	

	//Create a new thread
	Thread *execThread;
	execThread=new Thread("exec_Thread");
	
	#ifdef NETWORK

				
	mailboxNoGeneratorLock -> Acquire();
	execThread -> mailboxNo=mailboxNoGenerator++;				//Mailbox ID of user thread is set now
	mailboxNoGeneratorLock -> Release();				


	// Project 4 Code Starts	*************************************************************

	Thread *networkThread;		printf("network thread called\n");
	networkThread=new Thread("Network_Thread");

	mailboxNoGeneratorLock -> Acquire();
	networkThread -> mailboxNo=mailboxNoGenerator++;				//Mailbox ID of network thread is set now
	mailboxNoGeneratorLock -> Release();

	AddrSpace *addressSpaceForNetworkThread;

	addressSpaceForNetworkThread=new AddrSpace(f);				//CHECK **************
	addressSpaceForNetworkThread -> spaceID=processUniqueID++;		//The process's unique ID is the addrSpace's uniqueID

	addressSpaceForNetworkThread -> allocateStack();



	pInfoTable.noProcesses++;						//Number of processes in the system +=1
	pInfoTable.noThreads[processUniqueID]++;				//Number of threads for Current Process +=1


	networkThread -> processID=processUniqueID++;				//processUniqueID incremented for the next Process

	//Allocate the space created to this thread's space
	networkThread -> space=addressSpaceForNetworkThread;

	//Fork the new thread.I call it execThread
	networkThread -> Fork((VoidFunctionPtr)network_thread_function,0);


	// Project 4 Code Ends		*************************************************************	
	

	#endif


	
	//Update the process table and related data structures
	addressSpaceForExecThread -> allocateStack();				//8 pages for 1 thread's stack added to the AddrSpace

	pInfoTable.noProcesses++;						//Number of processes in the system +=1
	pInfoTable.noThreads[processUniqueID]++;				//Number of threads for Current Process +=1


	execThread -> processID=processUniqueID++;				//processUniqueID incremented for the next Process


	//Allocate the space created to this thread's space
	execThread -> space=addressSpaceForExecThread;


	pInfoLock -> Release();


	//Fork the new thread.I call it execThread
	execThread -> Fork((VoidFunctionPtr)exec_thread_function,0);


	//Write the space ID to the register 2
	return addressSpaceForExecThread -> spaceID;
}



void Exit_Syscall(int a)
{
	unsigned int x;			

	pInfoLock -> Acquire();


	//CASE 1: The Exiting thread is the last thread in Nachos
	if(pInfoTable.noProcesses == 1 && pInfoTable.noThreads[currentThread -> processID] == 1)   
   	{
		//Clear all the physical pages used in the AddrSpace of this process
		for(x=0; x < (currentThread -> space -> numPages); x++)
			currentThread -> space -> clearPhysicalPage(x);		//OR CALL DESTRUCTOR, CHECK !!!!	 	
			
		               
        	pInfoLock -> Release();
        
		DEBUG('x', "\n Removing last thread of Nachos\n");

		//Halting Now, as the last thread of Nachos is no more
		interrupt->Halt();
    	}


	//CASE 2: The Exiting thread is the last thread in a process
	else if(pInfoTable.noProcesses > 1 && pInfoTable.noThreads[currentThread -> processID] == 1)   //SHOULD BE 1, CHECK !!!
   	{
		pInfoTable.noProcesses--;					//Number of processes in the system -=1
		pInfoTable.noThreads[currentThread -> processID]=0;


		//Clear all the physical pages used in the AddrSpace of this process
		for(x=0; x < (currentThread -> space -> numPages); x++)
			currentThread -> space -> clearPhysicalPage(x);		//OR CALL DESTRUCTOR, CHECK !!!!	 	
	

		currentThread -> space = NULL;					//WHY NOT ABOVE CHECK 		
		               
        	pInfoLock -> Release();
        	
		
		DEBUG('x', "\n Removing last thread in a Process\n");

		//Current Thread's job is now over
		currentThread -> Finish();
    	}


	//CASE 3: The Exiting thread is just another normal thread in a process
	else 
   	{
		pInfoTable.noThreads[currentThread -> processID]--;

	
		pInfoLock -> Release();

		
		DEBUG('x', "\n Removing just another thread in a process\n");        

		//Current Thread's job is now over
		currentThread -> Finish();
	}
}


int CreateList_Syscall()
{
	listLock -> Acquire();


	List *newlyCreatedList=new List();

	int listID=-1;

	for (int i=0; i < MAX_LISTS; i++)
		if (osLists[i] == NULL)
		{
			osLists[i]=newlyCreatedList;
			listID=i;

			break;
		}

	listLock -> Release();

	DEBUG('x', "\nList Created");

	return listID;

}	


void AppendList_Syscall(int listID, int number) 
{
    List* tempList = osLists[listID];
    tempList -> Append((void*)number);

	DEBUG('x', "\nNumber %d appended to list", number);
}


int RemoveList_Syscall(int listID) 
{
	int number;
    List* tempList = osLists[listID];

	number=(int) tempList -> Remove();
	
	DEBUG('x', "\nNumber %d removed from list", number);

    return number;
}


int Rand_Syscall(int randNo)
{
	return (rand() % randNo);
}



// *****************CREATED ENDS HERE*********************


void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv=0; 	// the return value from a syscall

    if ( which == SyscallException ) {
	switch (type) {
	    default:
		DEBUG('a', "Unknown syscall - shutting down.\n");
	    case SC_Halt:
		DEBUG('a', "Shutdown, initiated by user program.\n");
		interrupt->Halt();
		break;
	    case SC_Create:
		DEBUG('a', "Create syscall.\n");
		Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Open:
		DEBUG('a', "Open syscall.\n");
		rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Write:
		DEBUG('a', "Write syscall.\n");
		Write_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
	    case SC_Read:
		DEBUG('a', "Read syscall.\n");
		rv = Read_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
	    case SC_Close:
		DEBUG('a', "Close syscall.\n");
		Close_Syscall(machine->ReadRegister(4));
		break;

// *****************CREATED STARTS HERE*********************

		case SC_Fork:
			DEBUG('a', "Fork syscall.\n");
			Fork_Syscall(machine->ReadRegister(4));
		break;		
	
		case SC_Exec:
			DEBUG('a', "Exec syscall.\n");
			rv = Exec_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
		

		case SC_Exit:
			DEBUG('a', "Exit Syscall.\n");
			Exit_Syscall(machine->ReadRegister(4));
		break;
		
	    
	   
		case SC_Yield:						//Implementation for the Yield() System Call
			DEBUG('x', "Current Thread gave away CPU using Yield.\n");
			currentThread -> Yield();
			break;
					
		case SC_CreateLock:					//Implementation for CreateLock() System Call
			DEBUG('a', "Create Lock.\n");
			rv=CreateLock_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			break;
		
		case SC_DestroyLock:					//Implementation for DestroyLock() System Call
			DEBUG('a', "Destroy Lock.\n");
			DestroyLock_Syscall(machine->ReadRegister(4));
			break;
			
		case SC_Acquire:					//Implementation for Acquire() System Call
			DEBUG('a', "Acquiring Lock.\n");
			Acquire_Syscall(machine->ReadRegister(4));
			break;
 
		case SC_Release:					//Implementation for Release() System Call
			DEBUG('a', "Releasing Lock.\n");
			Release_Syscall(machine->ReadRegister(4));
			break;



		case SC_CreateCondition:				//Implementation for CreateCondition() System Call
			DEBUG('a', "Create Condition Variable.\n");
			rv=CreateCondition_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			break;
		
		case SC_DestroyCondition:				//Implementation for DestroyCondition() System Call
			DEBUG('a', "Destroy Condition Variable.\n");
			DestroyCondition_Syscall(machine->ReadRegister(4));
			break;

		

		case SC_Wait:					//Implementation for Wait() System Call
			DEBUG('a', "Wait().\n");
			Wait_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			break;

		case SC_Signal:					//Implementation for Signal() System Call
			DEBUG('a', "Signal().\n");
			Signal_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			break;

		case SC_Broadcast:				//Implementation for Broadcast() System Call
			DEBUG('a', "Broadcast().\n");
			Broadcast_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			break;

		
		case SC_Print:					//Implementation for Print() System Call
			DEBUG('a', "Print().\n");
			
			int textToBePrinted;
	  		int num1, num2, num3;
	  		char *printfBuffer = new char[200+1];
	  
	  		textToBePrinted = machine->ReadRegister(4);
	  		num1 = machine->ReadRegister(5);
	  		num2 = machine->ReadRegister(6);
	  		num3 = machine->ReadRegister(7);
	  	

			if(!printfBuffer) 
	  	  		DEBUG('a',"Problem in allocating kernel buffer in Print\n");
	  	  
	  		if(copyin(textToBePrinted, 200, printfBuffer)==-1) 
			{
	    			DEBUG('a',"Cannot print the value passed to print\n");
	    			delete[] printfBuffer;
	  		}
	  		
			printfBuffer[200] = '\0';
	  		
			printf(printfBuffer,num1,num2,num3);
	 	 
			break;

		
		case SC_CreateList:				//Implementation for CreateList() System Call
			DEBUG('a', "CreateList().\n");
			rv=CreateList_Syscall();
			break;

		
		case SC_AppendList:				//Implementation for AppendList() System Call
			DEBUG('a', "AppendList().\n");
			AppendList_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			break;


		case SC_RemoveList:				//Implementation for RemoveList() System Call
			DEBUG('a', "RemoveList().\n");
			rv=RemoveList_Syscall(machine->ReadRegister(4));
			break;

		case SC_Rand:
			DEBUG('a', "Rand().\n");
			rv=Rand_Syscall(machine->ReadRegister(4));
			break;

		case SC_CreateMV:
			DEBUG('x', "Create Monitor Variable Syscall.\n");
			CreateMV_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
			break;
		
		case SC_SetMV:
			DEBUG('x', "Create Monitor Variable Syscall.\n");
			SetMV_Syscall(machine->ReadRegister(4),machine->ReadRegister(5),machine->ReadRegister(6));
			break;
		
		case SC_GetMV:
			DEBUG('x', "Create Monitor Variable Syscall.\n");
			GetMV_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
			break;
		
		case SC_IncrementMV:
			DEBUG('x', "Create Monitor Variable Syscall.\n");
			IncrementMV_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
			break;
		
		case SC_DecrementMV:
			DEBUG('x', "Create Monitor Variable Syscall.\n");
			DecrementMV_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
			break;

		case SC_DMV:
			DEBUG('x', "Create Monitor Variable Syscall.\n");
			DMV_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
			break;



	//   *****************CREATED ENDS HERE*********************

}

	// Put in the return value and increment the PC
	machine->WriteRegister(2,rv);
	machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
	machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
	machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
	return;
    } else {
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }
}

