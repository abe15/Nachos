#include "copyright.h"
#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"
#include "string.h"
#include "list.h"
#include "sys/time.h"

extern "C" {int bzero(char *, int); };
#define MAX_VARS_MV 500	//maximum variables
#define MAX_VARS_LC_CV 75	//maximum variables
#define ClientCharSize 600	//maximum variables


char * addTimeStamp(char * timeStamp,char * msg,int len);
void SendToAll(char * clients,int clientsLen,char * msg,int msgLen);

struct MyMessages
{
int mid;
int mbn;
char msg[MaxMailSize];
long int timeStamp;
};


struct State
{
int mid;
int mbn;
long int timeStamp;
int seqNo;

};


struct ServerLock			//structure for locks
{
	char name[25];			//storing the name of lock
	int lockID;				//storing the lock ID
	bool isLockFree;		//boolean variable to see lock is free or not
	int lockOwnerMID;		//storing client machine ID who owns the lock
	int lockOwnerMBN;		//storing client mailbox number 
	int qcount;				//lock usage counter
	List *msgList;			//list to store the message for client, used by server	
	List *midList;			//list to store the client machine ID, used by server
	List *mbnList;			//list to store the client mailbox number, used by server
};

struct ServerConditionVariable	//struct for condition variables
{
	char name[25];			//storing name of CV
	int cvID;				//storing CV ID
	int lockID;				//storing the lockID
	int lockMID;			//storing client machine ID
	int lockMBN;			//storing the client mailbox number
	int qcount;				//CV usage counter
	List *cvLmidList;		//list to store the client machine ID
	List *cvLmbnList;		//list to store the client mailbox number
};

struct Monitor			//structure for monitor variables
{
	char name[30];		//storing the name of monitor variable	
	int value;			//storing the value of monitor variable
	int monitorID;		//storing monitor variable ID
		
};


//---------------------------------------------------------------------------
// Server_CreateLock
// Handle Remote Procedure Call to create a lock
// Create the lock with the name in the user buffer  
//---------------------------------------------------------------------------
void Server_CreateLock(char *lockName, int clientMID, int clientMBN, ServerLock sLocks[], int *dlockCount)
{
	PacketHeader outPktHdr, inPktHdr;			//declaring objects of PacketHeader
    MailHeader outMailHdr, inMailHdr;			//declaring objects of MailHeader
	char *ack=" ";

    bool flag1, success, lockExist=false;		//booleans used in function
	int lockCount;
	lockCount=*dlockCount;
	
    outPktHdr.to = clientMID;			//storing the Machine ID of client to whom this packet is to send
    outMailHdr.to = clientMBN+1;			//storing the MailBox number of client to whom this packet is to send
	outMailHdr.from=currentThread->mailBoxNo;

	

	int p;					// for loop counter variable
				
	if(lockCount>0)				//checking if any lock exists.
	{
        for(p=0; p<lockCount; p++)
        {
            if( strcmp(sLocks[p].name, lockName) == 0 )		//comparing the name of lock(s) with the name passed 
            {   
                lockExist = true;					//check if lock already exists
				//printf("NT : %s already exists and the lock is %d\n",lockName,p);
                break;
            }
        }
	}
    if(lockCount>=MAX_VARS_LC_CV)	//checking if number of locks exists is greater or equal to MAX_SLOCKS
    {
		printf("NT : Error: Cannot create lock, the lockTable is full.\n");
        sprintf(ack, "%d", -1);		//Preparing ack to send to client, -1 indicates error in creating lock
    }
    
	if(lockExist)			//check if lock exists
	{
		sprintf(ack, "%d", sLocks[p].lockID);	//Preparing ack to send to client, ack now contains the lockID
		printf("NT : Error: Lock '%s' already created\n",lockName);
		lockExist = false;		
	}
	else				//if lock doesnt exist
    {
		strcpy(sLocks[lockCount].name,lockName); //copying the name passed in the name(part of struct) of lock
		sLocks[lockCount].lockID = lockCount;	//lockID is set equal to the lockCount
		sLocks[lockCount].isLockFree = true;	//shows that lock is free and not acquired
		sLocks[lockCount].lockOwnerMID = -1;	//No lock owner yet
		sLocks[lockCount].lockOwnerMBN = -1;	//No lock owner yet
		sLocks[lockCount].qcount = 0;			//usage counter of locks	
		sLocks[lockCount].msgList = new List;	//Initializing the message list for the lock
		sLocks[lockCount].midList = new List;	//Initializing the machineID list for the lock
		sLocks[lockCount].mbnList = new List;	//Initializing the mailbox number list for the lock
		//printf("NT : %s created\n",sLocks[lockCount].name);
		lockCount++;			//incrementing the lock count
		sprintf(ack, "%d",lockCount-1);		//Preparing ack to send to client, ack now contains the lockID
    }

	outMailHdr.length = strlen(ack) + 1;	//storing the length of ack data in Mail Header
	*dlockCount=lockCount;

	if(clientMID==postOffice->GetMachineID() && clientMBN==currentThread->mailBoxNo)
	{
		success = reliablePost->Send(outPktHdr, outMailHdr, ack); //sending the packet containing ack to client
		//printf("NT : Server Sending the Response to the Client\n");
		if ( !success ) 			//check to see if packet is sent
    	{
        	printf("NT : The postOffice Send failed.\n");
	        interrupt->Halt();
    	}
	}
}



//---------------------------------------------------------------------------
// Server_DestroyLock
// Handle Remote Procedure Call to delete a lock
// Delete the lock with the name in the user buffer  
//---------------------------------------------------------------------------
void Server_DestroyLock(int reqLock, int clientMID, int clientMBN, ServerLock sLocks[], int *dlockCount)
{
	char *ack=" ";
	PacketHeader outPktHdr, inPktHdr;		//declaring objects of PacketHeader
    MailHeader outMailHdr, inMailHdr;		//declaring objects of MailHeader
    int lockCount;
	lockCount=*dlockCount;
	
    bool flag1, success;
	
    outPktHdr.to = clientMID;			//storing the Machine ID of client to whom this packet is to send
    outMailHdr.to = clientMBN+1;			//storing the MailBox number of client to whom this packet is to send
	outMailHdr.from=currentThread->mailBoxNo;

	//printf("NT : The request is to delete the lockID %d\n",reqLock);

	if (reqLock < 0 || reqLock >= lockCount)	//check if requested lock id exist or not
    {
       	printf("NT : Error: Invalid lockID\n");
		sprintf(ack, "%d", -1);				//Preparing ack to send to client, -1 indicates error 
    }
	else if(sLocks[reqLock].lockID ==-1)	//check if lock is already destroyed
	{
		printf("NT : Error: The lock is already destroyed.\n");
		sprintf(ack, "%d", -1);			//Preparing ack to send to client, -1 indicates error 
	}
    else if (sLocks[reqLock].qcount > 0 || sLocks[reqLock].isLockFree == false)		//check if any one using or waiting for the lock
    {
       	printf("NT : cannot destroy lock because someone is waiting for it.\n");
		sprintf(ack, "%d", -1);		//Preparing ack to send to client, -1 indicates error 
    }
	else		//delete the lock
    {
		//printf("NT : Deleted %s \n",sLocks[reqLock].name);
		strcpy(sLocks[reqLock].name," ");		//delete name
		sLocks[reqLock].lockID = -1;			//-1 indicates lock does not exists
		sLocks[reqLock].isLockFree = false;
		sLocks[reqLock].lockOwnerMID = -1;		//no lock owner
		sLocks[reqLock].lockOwnerMBN = -1;		
		sprintf(ack, "%d", 1);		//Preparing ack to send to client, 1 indicates lock successfully deleted 
	}

    outMailHdr.length = strlen(ack) + 1;		//storing the length of ack data in Mail Header
	*dlockCount=lockCount;

	if(clientMID==postOffice->GetMachineID() && clientMBN==currentThread->mailBoxNo)
	{
		success = reliablePost->Send(outPktHdr, outMailHdr, ack); //sending the packet containing ack to client
		//printf("NT : Server Sending the Response to the Client\n");
		if ( !success ) 			//check to see if packet is sent
    	{
        	printf("NT : The postOffice Send failed.\n");
	        interrupt->Halt();
    	}
	}
}

//---------------------------------------------------------------------------
// Server_Acquire
// Handle the Remote Procedure Call 
// to acquire a lock
//---------------------------------------------------------------------------
void Server_Acquire(int reqLock, int clientMID, int clientMBN, ServerLock sLocks[], int *dlockCount)
{
	
	char *ack=" ";
	PacketHeader outPktHdr, inPktHdr;		//declaring objects of PacketHeader
    MailHeader outMailHdr, inMailHdr;		//declaring objects of MailHeader
	int lockCount;
	lockCount=*dlockCount;
	
    bool flag1, success;		//booleans used in function
	
	outPktHdr.to = clientMID;			//storing the Machine ID of client to whom this packet is to send
    outMailHdr.to = clientMBN+1;			//storing the MailBox number of client to whom this packet is to send
	outMailHdr.from=currentThread->mailBoxNo;
	
	if (reqLock < 0 || reqLock >= lockCount)	//check if requested lock id exist or not
    {
       	printf("NT : Error: invalid lockID.\n");
		sprintf(ack, "%d", -1);			//Preparing ack to send to client, -1 indicates error 
		outMailHdr.length = strlen(ack) + 1;	//storing the length of ack data in Mail Header
		success = reliablePost->Send(outPktHdr, outMailHdr, ack); 	//sending the packet containing ack to client
		//printf("NT : Server Sending the Response to the Client\n");
		if ( !success ) 	//check to see if packet is sent
		{
			printf("NT : The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
			interrupt->Halt();
		}
    }
	else if(sLocks[reqLock].lockID == -1)		//check if lock is deleted
	{
       	printf("NT : Error: The lock is already deleted, hence cann't acquire.\n");
		sprintf(ack, "%d", -1);			//Preparing ack to send to client, -1 indicates error
		outMailHdr.length = strlen(ack) + 1;		//storing the length of ack data in Mail Header
		success = reliablePost->Send(outPktHdr, outMailHdr, ack); 		//sending the packet containing ack to client
		//printf("NT : Server Sending the Response to the Client\n");
		if ( !success ) 			//check to see if packet is sent
		{
			printf("NT : The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
			interrupt->Halt();
		}
	}
	else if(sLocks[reqLock].lockOwnerMID == clientMID && sLocks[reqLock].lockOwnerMBN == clientMBN)  //check if lock owner wants to 
	{																						//acquire again
       	printf("NT : Error: The client already owns the lock, hence cann't acquire.\n");//,sLocks[reqLock].lockOwnerMID,clientMID,sLocks[reqLock].lockOwnerMBN,clientMBN);
       	sprintf(ack, "%d", -1);			//Preparing ack to send to client, -1 indicates error
		outMailHdr.length = strlen(ack) + 1;
		success = reliablePost->Send(outPktHdr, outMailHdr, ack); 		//sending the packet containing ack to client
		//printf("NT : Server Sending the Response to the Client\n");
		if ( !success ) 		//check to see if packet is sent
		{
			printf("NT : The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
			interrupt->Halt();
		}
	}
	else		//if not the lock owner
	{
		if (sLocks[reqLock].isLockFree)		//check if lock is free or not
		{
			//printf("NT : The request is to acquire %s whose lockID %d\n",sLocks[reqLock].name,reqLock);
			sLocks[reqLock].isLockFree = false;		
			sLocks[reqLock].lockOwnerMID = clientMID;		//save the client machhine ID 
			sLocks[reqLock].lockOwnerMBN = clientMBN;		//save client mail box number
			sprintf(ack, "%d", 1);			//Preparing ack to send to client, 1 indicates success in acquire lock
			outMailHdr.length = strlen(ack) + 1;		//storing the length of ack data in Mail Header


			if(clientMID==postOffice->GetMachineID() && clientMBN==currentThread->mailBoxNo)
			{
				success = reliablePost->Send(outPktHdr, outMailHdr, ack); //sending the packet containing ack to client
				//printf("NT : Server Sending the Response to the Client\n");
				if ( !success ) 			//check to see if packet is sent
				{
					printf("NT : The postOffice Send failed.\n");
					interrupt->Halt();
				}
			}
		}
		else		//if lock is not free, client request goes in queue
		{
			//printf("NT : The request is going in the queue.\n");
			sprintf(ack, "%d", 1);		
			sLocks[reqLock].msgList->Append((void *)ack);		//client request message is saved in list
			sLocks[reqLock].midList->Append((void *)clientMID);	//client machineID is saved in list
			sLocks[reqLock].mbnList->Append((void *)clientMBN); //CLIENT MAILBOX NUMBER IS SAVED IN LIST
			sLocks[reqLock].qcount++;		//incrementing the usage counter
		}
	}	
	*dlockCount=lockCount;

}

//---------------------------------------------------------------------------
// Server_Release
// Handle the Remote Procedure Call 
// to release a lock
//---------------------------------------------------------------------------
void Server_Release(int reqLock, int clientMID, int clientMBN, int number, ServerLock sLocks[], int *dlockCount)
{

	char *ack=" ";
	PacketHeader outPktHdr, inPktHdr, outPktHdr_client;			//same as above 
    MailHeader outMailHdr, inMailHdr, outMailHdr_client;
	int lockCount;
	lockCount=*dlockCount;
	
    bool success;			//boolean used in function
	int flag=0;				//initializing the flag to  0
  
	outPktHdr.to = clientMID;			//storing the Machine ID of client to whom this packet is to send
    outMailHdr.to = clientMBN+1;			//storing the MailBox number of client to whom this packet is to send
	outMailHdr.from=currentThread->mailBoxNo;

	//printf("NT : In release lock the number is %d \n",number);
	if (reqLock < 0 || reqLock >= lockCount)		//check if requested lock id exist or not
    {
        printf("NT : Error: invalid lockID.\n");
		sprintf(ack, "%d", -1);				//Preparing ack to send to client, -1 indicates error
		flag=1;					
    }
	else if(sLocks[reqLock].lockID == -1)	//check if lock is destroyed
	{
        printf("NT : Error: The lock is already destroyed.\n");
		sprintf(ack, "%d", -1);				//Preparing ack to send to client, -1 indicates error
		flag=1;
	}
	else if(sLocks[reqLock].lockOwnerMID != clientMID || sLocks[reqLock].lockOwnerMBN != clientMBN)  // ---- problem ----
	{
		printf("NT : Error: The client is not the lock owner.\n");	
		sprintf(ack, "%d", -1);				//Preparing ack to send to client, -1 indicates error
		flag=1;
	}
	else 
	{
		if(sLocks[reqLock].qcount==0)		//check if no one is using the lock
		{
			sLocks[reqLock].isLockFree = true;	//free the lock
            sLocks[reqLock].lockOwnerMID = -1;	//ownership of lock is removed
			sLocks[reqLock].lockOwnerMBN = -1;	//ownership of lock is removed
			sprintf(ack, "%d", 1);			//Preparing ack to send to client, 1 indicates successful release operatin
			
			//printf("NT : I am here\n");
			if(number==1)	
			{
				outMailHdr.length = strlen(ack) + 1;		//storing the length of ack data in Mail Header
				if(clientMID==postOffice->GetMachineID() && clientMBN==currentThread->mailBoxNo)
				{
					success = reliablePost->Send(outPktHdr, outMailHdr, ack); //sending the packet containing ack to client
					//printf("NT : Server Sending the Response to the Client\n");
					if ( !success ) 			//check to see if packet is sent
					{
						printf("NT : The postOffice Send failed.\n");
						interrupt->Halt();
					}
				}
			}
		}
		else
		{
			if(number==1)
			{
				outMailHdr.length = strlen(ack) + 1;	//storing the length of ack data in Mail Header
				if(clientMID==postOffice->GetMachineID() && clientMBN==currentThread->mailBoxNo)
				{
					success = reliablePost->Send(outPktHdr, outMailHdr, ack); //sending the packet containing ack to client
					//printf("NT : Server Sending the Response to the Client\n");
					if ( !success ) 			//check to see if packet is sent
					{
						printf("NT : The postOffice Send failed.\n");
						interrupt->Halt();
					}
				}
		
			}

			sLocks[reqLock].isLockFree = false;
			ack=(char *)sLocks[reqLock].msgList->Remove();
			sLocks[reqLock].lockOwnerMID = (int)sLocks[reqLock].midList->Remove();
			sLocks[reqLock].lockOwnerMBN = (int)sLocks[reqLock].mbnList->Remove();
			sLocks[reqLock].qcount--;
			outPktHdr.to = sLocks[reqLock].lockOwnerMID;
			outMailHdr.to = sLocks[reqLock].lockOwnerMBN+1;
			outMailHdr.from=currentThread->mailBoxNo;
			outMailHdr.length = strlen(ack) + 1;		//storing the length of ack data in Mail Header
			if(sLocks[reqLock].lockOwnerMID==postOffice->GetMachineID() && sLocks[reqLock].lockOwnerMBN==currentThread->mailBoxNo)
			{
				success = reliablePost->Send(outPktHdr, outMailHdr, ack); //sending the packet containing ack to client
				//printf("NT : Server Sending the Response to the Client\n");
				if ( !success ) 			//check to see if packet is sent
				{
					printf("NT : The postOffice Send failed.\n");
					interrupt->Halt();
				}
			}
		
		}
	}
	
	if(flag==1)
	{
		outMailHdr.length = strlen(ack) + 1;
		success = reliablePost->Send(outPktHdr, outMailHdr, ack); 	//sending the packet containing ack to client
		//printf("NT : Server Sending the Response to the Client\n");
		if ( !success ) 			//check to see if packet is sent
		{
			printf("NT : The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
			interrupt->Halt();
		}
	}
	*dlockCount=lockCount;
}

//---------------------------------------------------------------------------
// Server_CreateCondition
// Handle the Remote Procedure Call to create a new CV
// Create the CV with the name in the user buffer 
//---------------------------------------------------------------------------
void Server_CreateCondition(char *cvName, int clientMID, int clientMBN,ServerConditionVariable sCV[], int *tcvCount)
{
	char *ack=" ";
	int cvCount=*tcvCount;
	
	PacketHeader outPktHdr, inPktHdr;		//declaring objects of PacketHeader
    MailHeader outMailHdr, inMailHdr;		//declaring objects of MailHeader

    bool flag1, success, cvExist=false;
	
	outPktHdr.to = clientMID;			//storing the Machine ID of client to whom this packet is to send
    outMailHdr.to = clientMBN+1;			//storing the MailBox number of client to whom this packet is to send
	outMailHdr.from=currentThread->mailBoxNo;
	
	int p;					// for loop counter variable
				
	if(cvCount>0)		//check if any CV exists
	{
        for(p=0; p<cvCount; p++)
        {
            //printf("NT : %s  %s \n",sCV[p].name, cvName);
			if( strcmp(sCV[p].name, cvName) == 0 )			//comparing the name of CV(s) with the name passed 
            {   
                cvExist = true;			//changing the boolean value to true
				//printf("NT : %s already exists and the cv is %d\n",cvName,p);
                break;
            }
        }
	}
    if(cvCount>=MAX_VARS_LC_CV)		//check if number of CV is equal to or more than 200
    {
		printf("NT : Error: Cannot create cv, the cvTable is full.\n");
        sprintf(ack, "%d", -1);			//Preparing ack to send to client, -1 indicates error
    }
    
	if(cvExist)			//if CV is present
	{

		sprintf(ack, "%d", sCV[p].cvID);	//Preparing ack to send to client, ack now contains the CV id
		printf("NT : Error: condition '%s' already created\n",cvName);
		cvExist = false;	

	}
	else		//if CV does not exist
    {
		strcpy(sCV[cvCount].name,cvName);	//copying the name passed in the name(part of struct) of CV
		sCV[cvCount].cvID = cvCount;		//CVid is set equal to cvCount
		sCV[cvCount].lockID = -1;			//lockid is not set 
		sCV[cvCount].lockMID = -1;			//No lock owner yet
		sCV[cvCount].lockMBN = -1;			//No lock owner yet
		sCV[cvCount].qcount = 0;			//usage counter of CVs
		sCV[cvCount].cvLmidList = new List;	//Initializing the machineID list for the CV
		sCV[cvCount].cvLmbnList = new List;	//Initializing the mailbox list for the CV
		//printf("NT : %s created\n",sCV[cvCount].name);
		cvCount++;				//incrementing the number of CV created
		sprintf(ack, "%d",cvCount-1);		//Preparing ack to send to client, ack now contains the CV id
    }
	outMailHdr.length = strlen(ack) + 1;	//storing the length of ack data in Mail Header
	
	if(clientMID==postOffice->GetMachineID() && clientMBN==currentThread->mailBoxNo)
	{
		success = reliablePost->Send(outPktHdr, outMailHdr, ack); 	//sending the packet containing ack to client
		//printf("NT : Server Sending the Response to the Client\n");
 
		if ( !success ) 			//check to see if packet is sent
		{
			printf("NT : The postOffice Send failed.\n");
			interrupt->Halt();
		}
	}
	*tcvCount=cvCount;
}

//---------------------------------------------------------------------------
// Server_DestroyCondition
// Handle the Remote Procedure Call to delete a new CV
//  
//---------------------------------------------------------------------------
void Server_DestroyCV(int reqCV, int clientMID, int clientMBN,ServerConditionVariable sCV[], int *tcvCount)
{
	char *ack="Ack msg";
	int cvCount=*tcvCount;
	PacketHeader outPktHdr, inPktHdr;		//declaring objects of PacketHeader
    MailHeader outMailHdr, inMailHdr;		//declaring objects of MailHeader

    bool flag1, success;			//booleans used in function
	
	outPktHdr.to = clientMID;			//storing the Machine ID of client to whom this packet is to send
    outMailHdr.to = clientMBN+1;			//storing the MailBox number of client to whom this packet is to send
	outMailHdr.from=currentThread->mailBoxNo;


	//printf("NT : The request is to delete the cvID %d\n",reqCV);
	if (reqCV < 0 || reqCV >= cvCount)		//check the CV id 
    {
       	printf("NT : Error: invalid cvID.\n");
		sprintf(ack, "%d", -1);			//Preparing ack to send to client, -1 indicates error
    }
    else if (sCV[reqCV].cvID == -1)		//check If CV id exist
    {
       	printf("NT : Error: cannot destroy CV because it is already destroyed.\n");
		sprintf(ack, "%d", -1);			//Preparing ack to send to client, -1 indicates error
    }
    else if (sCV[reqCV].qcount > 0)		//check the CV usage counter
    {
       	printf("NT : Error: cannot destroy CV because someone is waiting for it.\n");
		sprintf(ack, "%d", -1);			//Preparing ack to send to client, -1 indicates error
    }
	else		//delete the CV
    {
		//printf("NT : Deleted %s \n",sCV[reqCV].name);
		strcpy(sCV[reqCV].name," ");		//delete name
		sCV[reqCV].cvID = -1;				//delete the CV id
		sCV[reqCV].lockID = -1;				//delete lock id
		sCV[reqCV].lockMID = -1;			//remove ownership	
		sCV[reqCV].lockMBN = -1;			//remove ownership
		sprintf(ack, "%d", 1);				//Preparing ack to send to client, 1 indicates successful deletion
		
	}
    outMailHdr.length = strlen(ack) + 1;		//storing the length of ack data in Mail Header
	if(clientMID==postOffice->GetMachineID() && clientMBN==currentThread->mailBoxNo)
	{
		success = reliablePost->Send(outPktHdr, outMailHdr, ack); 	//sending the packet containing ack to client
		//printf("NT : Server Sending the Response to the Client\n");
 
		if ( !success ) 			//check to see if packet is sent
		{
			printf("NT : The postOffice Send failed.\n");
			interrupt->Halt();
		}
	}
	*tcvCount=cvCount;

}


//---------------------------------------------------------------------------
// Server_WaitCV
// Handle the Remote Procedure Call to wait on a CV
//---------------------------------------------------------------------------
int Server_WaitCV(int reqCV, int reqLock, int clientMID,int clientMBN,ServerLock sLocks[], int *dlockCount,ServerConditionVariable sCV[], int *tcvCount)
{
	PacketHeader outPktHdr, inPktHdr;		//declaring objects of PacketHeader
    MailHeader outMailHdr, inMailHdr;	//declaring objects of MailHeader

    bool flag1, success;

	int cvCount=*tcvCount;
	int lockCount=*dlockCount;
	
	outPktHdr.to = clientMID;			//storing the Machine ID of client to whom this packet is to send
    outMailHdr.to = clientMBN+1;			//storing the MailBox number of client to whom this packet is to send
	outMailHdr.from=currentThread->mailBoxNo;


	//printf("NT : Client requests for the WaitCV for lockID %d and cvID %d\n",reqCV,reqLock);

    if (reqCV<0 || reqCV>= cvCount)		//check the condition variable id
    {
      	printf("NT : Error: invalid cvID\n");
		return -1;
    }
    if (sCV[reqCV].cvID == -1)		//check if CV is deleted 
    {
       	printf("NT : Error: invalid cv because cv already deleted.\n");
    	return -1;
	}
    if ( reqLock < 0 || reqLock >= lockCount)	//check the lock id
    {
       	printf("NT : Error: invalid lockID.\n");
		return -1;
    }
	if (sLocks[reqLock].lockID == -1)		//check if lock exist
	{
       	printf("NT : Error: Lock does not exist.\n");
		return -1;
	}
	if(sCV[reqCV].lockID == -1)				// if first person to call the wait
	{
		sCV[reqCV].lockID = sLocks[reqLock].lockID;		//save the lock id in CV data
	}
	if (sCV[reqCV].lockID != sLocks[reqLock].lockID )	//check If locks match
	{
       	printf("NT : Error: Wrong Lock used.\n");
		return -1;
	}
	if (sLocks[reqLock].lockOwnerMID != clientMID || sLocks[reqLock].lockOwnerMBN != clientMBN)	//check if machine id's match
    {
		printf("NT : Error: your machine id %d != %d or mailBoxNumber %d != %d\n",sLocks[reqLock].lockOwnerMID,clientMID,sLocks[reqLock].lockOwnerMBN,clientMBN);
    	return -1;
    }
	
		sCV[reqCV].qcount++;				//incrementing the usage counter
											//sCV[reqCV].lockID = reqLock;
		sCV[reqCV].lockMID = clientMID;		//saving the client machine ID
		sCV[reqCV].lockMBN = clientMBN;		//saving the client mailbox number
	
		sCV[reqCV].cvLmidList->Append((void *) clientMID);	//client request is saved in cv list
		sCV[reqCV].cvLmbnList->Append((void *) clientMBN);	//client request is saved in cv list

		*tcvCount=cvCount;
		*dlockCount=lockCount;
		
		Server_Release(reqLock,clientMID,clientMBN,0,sLocks,dlockCount);	
		return 1;

}



//---------------------------------------------------------------------------
// Server_SignalCV
// Handle the Remote Procedure Call to signal a waiter on a CV
//---------------------------------------------------------------------------
int Server_SignalCV(int reqCV, int reqLock, int clientMID,int clientMBN,ServerLock sLocks[], int *dlockCount,ServerConditionVariable sCV[], int *tcvCount)
{
	char *ack=" ";
	PacketHeader outPktHdr, inPktHdr;		//declaring objects of PacketHeader
    MailHeader outMailHdr, inMailHdr;		//declaring objects of MailHeader

    bool flag1, success;			//booleans used in function
	
	int cvCount=*tcvCount;
	int lockCount=*dlockCount;
	
	outPktHdr.to = clientMID;			//storing the Machine ID of client to whom this packet is to send
    outMailHdr.to = clientMBN+1;			//storing the MailBox number of client to whom this packet is to send
	outMailHdr.from=currentThread->mailBoxNo;


	//printf("NT : Client requests for the SignalCV for lockID %d and cvID %d\n",reqCV,reqLock);

    if (reqCV<0 || reqCV>= cvCount)		//check the condition variable id	
    {
      	printf("NT : Error: invalid cvID.\n");
		return -1;
    }
    if (sCV[reqCV].cvID == -1)		//check if CV is deleted 
    {
       	printf("NT : Error: invalid cv because already deleted.\n");
    	return -1;
	}
    if ( reqLock < 0 || reqLock >= lockCount)	//check the lock id
    {
       	printf("NT : Error: invalid lockID.\n");
		return -1;
    }
	if (sLocks[reqLock].lockID == -1)		//check if lock exist
	{	
       	printf("NT : Error: Lock does not exist.\n");
		return -1;
	}
    if (sCV[reqCV].qcount < 1)		//check the usage counter,i.e, the number of waiters
    {
       	printf("NT : Error: No one is waiting in the queue. '%s'\n",sCV[reqCV].name);
    	return -1;
	}
	if (sLocks[reqLock].lockID != sCV[reqCV].lockID)	//check If locks match
	{
       	printf("NT : Error: Wrong Lock used.\n");
		return -1;
	}
    if (sLocks[reqLock].lockOwnerMID != clientMID || sLocks[reqLock].lockOwnerMBN != clientMBN) //check if machine id's match
    {
		printf("NT : Error: your machine id %d != %d or mailBoxNumber %d != %d\n",sLocks[reqLock].lockOwnerMID,clientMID,sLocks[reqLock].lockOwnerMBN,clientMBN);
    	return -1;
    }
	
	int cmid=(int)sCV[reqCV].cvLmidList->Remove();		//removing the waiting client machine ID from the cvLmidLidt
	int cmbn=(int)sCV[reqCV].cvLmbnList->Remove();		//removing the waiting client mailbox number from the cvLmbnList
	sCV[reqCV].qcount--;		//decrement the CV usage count,i.e, number of waiters

	*tcvCount=cvCount;
	*dlockCount=lockCount;
	
	Server_Acquire(reqLock, cmid, cmbn,sLocks,dlockCount);
	
	return 1;
}

//---------------------------------------------------------------------------
// Server_BroadcastCV
// Handle the Remote Procedure Call to broadcast all waiters 
//---------------------------------------------------------------------------
int Server_BroadcastCV(int reqCV, int reqLock, int clientMID,int clientMBN, ServerLock sLocks[], int *dlockCount,ServerConditionVariable sCV[], int *tcvCount)
{
	char *ack="Ack msg";
	PacketHeader outPktHdr, inPktHdr;		//declaring objects of PacketHeader
    MailHeader outMailHdr, inMailHdr;		//declaring objects of MailHeader
	
    bool flag1, success;			//booleans used in function

	
	int cvCount=*tcvCount;
	int lockCount=*dlockCount;
	
	outPktHdr.to = clientMID;			//storing the Machine ID of client to whom this packet is to send
    outMailHdr.to = clientMBN+1;			//storing the MailBox number of client to whom this packet is to send
	outMailHdr.from=currentThread->mailBoxNo;


	//printf("NT : Client requests for the BroadcastCV for lockID %d and cvID %d\n",reqCV,reqLock);
    if (reqCV<0 || reqCV>= cvCount)		//check the condition variable id
    {
      	printf("NT : Error: invalid cvID\n");
		return -1;
    }
    if (sCV[reqCV].cvID == -1)		//check if CV is deleted 
    {
       	printf("NT : Error: invalid cv because already deleted\n Returning\n");
    	return -1;
	}
    if ( reqLock < 0 || reqLock >= lockCount)	//check the lock id
    {
       	printf("NT : Error: invalid LockID.\n");
		return -1;
    }
	if (sLocks[reqLock].lockID == -1)		//check if lock exist
	{
       	printf("NT : Error: Lock does not exist.\n");
		return -1;
	}
   /* if (sCV[reqCV].qcount < 1)		//check if any waiters
    {
       	printf("NT : Error: No one is waiting in the queue.\n");
    	return -1;
	}*/
	if (sLocks[reqLock].lockID != sCV[reqCV].lockID)	//check If locks match
	{
       	printf("NT : Error: Wrong Lock used.\n");
		return -1;
	}
    if (sLocks[reqLock].lockOwnerMID != clientMID || sLocks[reqLock].lockOwnerMBN != clientMBN)	//check if machine id's match
    {
		printf("NT : Error: your machine id %d != %d or mailBoxNumber %d != %d\n",sLocks[reqLock].lockOwnerMID,clientMID,sLocks[reqLock].lockOwnerMBN,clientMBN);
    	return -1;
    }

	int cmid, cmbn;		//variables used to store client machine id and mailbox number
	while(sCV[reqCV].qcount!=0)	
	{
		cmid=(int)sCV[reqCV].cvLmidList->Remove();		//removing the waiting client machine ID from the cvLmidLidt
		cmbn=(int)sCV[reqCV].cvLmbnList->Remove();		//removing the waiting client mailbox number from the cvLmbnLidt
		sCV[reqCV].qcount--;	//decrementing the waiters


		*tcvCount=cvCount;
		*dlockCount=lockCount;
		Server_Acquire(reqLock, cmid, cmbn,sLocks,dlockCount);

	}
	return 1;
}


//---------------------------------------------------------------------------
// Server_CreateMonitor
// Remote Procedure Call to create Monitor Variable
// Create the Monitor Variable with the name in the user buffer 
//---------------------------------------------------------------------------
void Server_CreateMonitor(char *mName, int clientMID, int clientMBN, int len, Monitor mVariable[], int *tmonitorCount,int value)
{
	char *ack="Ack msg";		//acknowlegement data to be send to client
	
	PacketHeader outPktHdr, inPktHdr;		//declaring objects of PacketHeader
	
    MailHeader outMailHdr, inMailHdr;	//declaring objects of MailHeader
	
	bool flag1, success, monitorExist=false;		//booleans used in function

	int monitorCount=*tmonitorCount;

	outPktHdr.to = clientMID;			//storing the Machine ID of client to whom this packet is to send
    outMailHdr.to = clientMBN+1;			//storing the MailBox number of client to whom this packet is to send
	outMailHdr.from=currentThread->mailBoxNo;

	
	int i;		//variable used in loop

	if(monitorCount > 0)	//checking if any monitor variable exists.
	{
		for(i=0; i < monitorCount; i++)
		{
			if( strcmp(mVariable[i].name, mName) == 0 )		//comparing the name of monitor variable(s) with the name passed 
			{
			//printf("NT : %s already created.\n",mName);
			
			monitorExist = true;			//boolean to show if monitor variable exist or not
			
			break;
			
			}
		
		}
	}

	if(monitorCount >= MAX_VARS_MV)		//checking if number of monitor variable exists is greater or equal to MAX_MONITOR
	{
		//printf("NT : Cannot create monitor variable\n");
	
		sprintf(ack, "%d", -1);   		//Preparing ack to send to client, -1 indicates error in creating monitor variable
	}
	
	if(monitorExist)		//check if monitor variable exists
	{
		sprintf(ack, "%d", mVariable[i].monitorID);		//Preparing ack to send to client
		printf("NT : Error: Monitor '%s' already created\n",mName);
		monitorExist = false;		//changing back the value to false for next iteration
	}

	else				//if monitor variable doesnt exists
	{
		strcpy(mVariable[monitorCount].name,mName);		//copying the name passed in the name(part of struct) of monitor variable 
		
		mVariable[monitorCount].value = value;			//Initializing the value to -1, it means value is not Set yet.
		
		mVariable[monitorCount].monitorID = monitorCount;
		
		//printf("NT : %s created\n",mVariable[monitorCount].name);
		
		monitorCount++;				//incrementing the number of monitor variables created
		
		sprintf(ack, "%d", monitorCount-1);		//Preparing ack to send to client, 1 indicates monitor variable created or exists
	}
	
	outMailHdr.length = strlen(ack) + 1;	//storing the length of ack data in Mail Header

	*tmonitorCount=monitorCount;
                
	if(clientMID==postOffice->GetMachineID() && clientMBN==currentThread->mailBoxNo)
	{
		success = reliablePost->Send(outPktHdr, outMailHdr, ack); //sending the packet containing ack to client
		//printf("NT : Server Sending the Response to the Client\n");
		if ( !success ) 			//check to see if packet is sent
		{
			printf("NT : The postOffice Send failed.\n");
			interrupt->Halt();
		}
	}

}


//---------------------------------------------------------------------------
// Server_SetMonitor
// Remote Procedure Call to set Monitor Variable
// Set the value of Monitor Variable with the value passed  
//---------------------------------------------------------------------------
void Server_SetMonitor(int index, int clientMID, int clientMBN, int mValue, Monitor mVariable[], int *tmonitorCount)
{
	char *ack="Ack msg";		//acknowlegement to be send to client	
	PacketHeader outPktHdr, inPktHdr;		//declaring objects of PacketHeader	
    MailHeader outMailHdr, inMailHdr;	//declaring objects of MailHeader	
	bool flag1, success, monitorExist=false;		//booleans used in function

	int monitorCount=*tmonitorCount;

	outPktHdr.to = clientMID;			//storing the Machine ID of client to whom this packet is to send
    outMailHdr.to = clientMBN+1;			//storing the MailBox number of client to whom this packet is to send
	outMailHdr.from=currentThread->mailBoxNo;
	
	if (index < 0 || index >= monitorCount)	//check if requested lock id exist or not
    {
       	printf("NT : Error: invalid monitorID.\n");
		sprintf(ack, "%d", -1);			//Preparing ack to send to client, -1 indicates error 
		outMailHdr.length = strlen(ack) + 1;	//storing the length of ack data in Mail Header

		*tmonitorCount=monitorCount;
                
		if(clientMID==postOffice->GetMachineID() && clientMBN==currentThread->mailBoxNo)
		{
			success = reliablePost->Send(outPktHdr, outMailHdr, ack); //sending the packet containing ack to client
			//printf("NT : Server Sending the Response to the Client\n");
			if ( !success ) 			//check to see if packet is sent
			{
				printf("NT : The postOffice Send failed.\n");
				interrupt->Halt();
			}
		}

    }
	/*
	if(mVariable[index].monitorID == -1)		//check if monitor is deleted
	{
       	printf("NT : Error: The monitor is deleted.\n");
		sprintf(ack, "%d", -1);			//Preparing ack to send to client, -1 indicates error
		outMailHdr.length = strlen(ack) + 1;		//storing the length of ack data in Mail Header
		
		*tmonitorCount=monitorCount;
						
		if(clientMID==postOffice->GetMachineID() && clientMBN==currentThread->mailBoxNo)
		{
			success = reliablePost->Send(outPktHdr, outMailHdr, ack); //sending the packet containing ack to client
			//printf("NT : Server Sending the Response to the Client\n");
			if ( !success ) 			//check to see if packet is sent
			{
				printf("NT : The postOffice Send failed.\n");
				interrupt->Halt();
			}
		}
	}
    else
	{*/
		//printf("NT : Monitor variable exists.\n");			
		mVariable[index].value = mValue;		//Storing the value passed in the value of MV			
		//monitorExist=true;					//boolean to show if monitor variable value is set or not			
		//printf("NT : Monitor variable value is set to %d\n",mVariable[index].value);
		sprintf(ack, "%d", 1);
		outMailHdr.length = strlen(ack) + 1;		//storing the length of ack data in Mail Header
		*tmonitorCount=monitorCount;
						
		if(clientMID==postOffice->GetMachineID() && clientMBN==currentThread->mailBoxNo)
		{
			success = reliablePost->Send(outPktHdr, outMailHdr, ack); //sending the packet containing ack to client
			//printf("NT : Server Sending the Response to the Client\n");
			if ( !success ) 			//check to see if packet is sent
			{
				printf("NT : The postOffice Send failed.\n");
				interrupt->Halt();
			}
		}
	/*}*/
}

//---------------------------------------------------------------------------
// Server_GetMonitor
// Remote Procedure Call to Get Monitor Variable
// Get the value of Monitor Variable   
//---------------------------------------------------------------------------
void Server_GetMonitor(int index, int clientMID, int clientMBN,Monitor mVariable[], int *tmonitorCount)
{

	int val;		//variable to store the value of MV and pass it on to client	
	char *ack="Ack msg";
	
	PacketHeader outPktHdr, inPktHdr;		//declaring objects of PacketHeader	
    MailHeader outMailHdr, inMailHdr;	//declaring objects of MailHeader	
	bool flag1, success, monitorExist=false;

	int monitorCount=*tmonitorCount;

	outPktHdr.to = clientMID;			//storing the Machine ID of client to whom this packet is to send
    outMailHdr.to = clientMBN+1;			//storing the MailBox number of client to whom this packet is to send
	outMailHdr.from=currentThread->mailBoxNo;

	int i;		//variable used in loop

	if (index < 0 || index >= monitorCount)	//check if requested lock id exist or not
    {
       	printf("NT : Error: invalid monitorID.\n");
		sprintf(ack, "%d", -1);			//Preparing ack to send to client, -1 indicates error 
		outMailHdr.length = strlen(ack) + 1;	//storing the length of ack data in Mail Header
		*tmonitorCount=monitorCount;
						
		if(clientMID==postOffice->GetMachineID() && clientMBN==currentThread->mailBoxNo)
		{
			success = reliablePost->Send(outPktHdr, outMailHdr, ack); //sending the packet containing ack to client
			//printf("NT : Server Sending the Response to the Client\n");
			if ( !success ) 			//check to see if packet is sent
			{
				printf("NT : The postOffice Send failed.\n");
				interrupt->Halt();
			}
		}
    }
	/*if(mVariable[index].monitorID == -1)		//check if monitor is deleted
	{
       	printf("NT : Error: The monitor is deleted.\n");
		sprintf(ack, "%d", -1);			//Preparing ack to send to client, -1 indicates error
		outMailHdr.length = strlen(ack) + 1;		//storing the length of ack data in Mail Header
		*tmonitorCount=monitorCount;
						
		if(clientMID==postOffice->GetMachineID() && clientMBN==currentThread->mailBoxNo)
		{
			success = reliablePost->Send(outPktHdr, outMailHdr, ack); //sending the packet containing ack to client
			printf("NT : Server Sending the Response to the Client\n");
			if ( !success ) 			//check to see if packet is sent
			{
				printf("NT : The postOffice Send failed.\n");
				interrupt->Halt();
			}
		}
	}
    else
	{*/
		//printf("NT : Monitor variable exists.\n");			
		//printf("NT : Monitor variable value is %d\n",mVariable[index].value);
		//monitorExist=true;					//boolean to show if monitor variable value is set or not			
		val = mVariable[index].value;
		sprintf(ack, "%d", val);
		outMailHdr.length = strlen(ack) + 1;		//storing the length of ack data in Mail Header
		*tmonitorCount=monitorCount;
						
		if(clientMID==postOffice->GetMachineID() && clientMBN==currentThread->mailBoxNo)
		{
			success = reliablePost->Send(outPktHdr, outMailHdr, ack); //sending the packet containing ack to client
			//printf("NT : Server Sending the Response to the Client\n");
			if ( !success ) 			//check to see if packet is sent
			{
				printf("NT : The postOffice Send failed.\n");
				interrupt->Halt();
			}
		}
	/*}	*/
}


void Server_DestroyMonitor(int index, int clientMID, int clientMBN, Monitor mVariable[], int *tmonitorCount)
{

	char *ack="Ack msg";	
	PacketHeader outPktHdr, inPktHdr;	
    MailHeader outMailHdr, inMailHdr;	
	bool flag1, success, monitorExist=false;

	int monitorCount=*tmonitorCount;

	outPktHdr.to = clientMID;			//storing the Machine ID of client to whom this packet is to send
    outMailHdr.to = clientMBN+1;			//storing the MailBox number of client to whom this packet is to send
	outMailHdr.from=currentThread->mailBoxNo;
	

	int i;

	if (index < 0 || index >= monitorCount)	//check if requested lock id exist or not
    {
       	printf("NT : Error: invalid monitorID.\n");
		sprintf(ack, "%d", -1);			//Preparing ack to send to client, -1 indicates error 
		outMailHdr.length = strlen(ack) + 1;	//storing the length of ack data in Mail Header
		*tmonitorCount=monitorCount;
						
		if(clientMID==postOffice->GetMachineID() && clientMBN==currentThread->mailBoxNo)
		{
			success = reliablePost->Send(outPktHdr, outMailHdr, ack); //sending the packet containing ack to client
			//printf("NT : Server Sending the Response to the Client\n");
			if ( !success ) 			//check to see if packet is sent
			{
				printf("NT : The postOffice Send failed.\n");
				interrupt->Halt();
			}
		}
    }
	if(mVariable[index].monitorID == -1)		//check if monitor is deleted
	{
       	printf("NT : Error: The monitor is already been deleted.\n");
		sprintf(ack, "%d", -1);			//Preparing ack to send to client, -1 indicates error
		outMailHdr.length = strlen(ack) + 1;		//storing the length of ack data in Mail Header
		*tmonitorCount=monitorCount;
						
		if(clientMID==postOffice->GetMachineID() && clientMBN==currentThread->mailBoxNo)
		{
			success = reliablePost->Send(outPktHdr, outMailHdr, ack); //sending the packet containing ack to client
			//printf("NT : Server Sending the Response to the Client\n");
			if ( !success ) 			//check to see if packet is sent
			{
				printf("NT : The postOffice Send failed.\n");
				interrupt->Halt();
			}
		}
	}
	else
	{
		
		//printf("NT : Destroy Monitor variable.\n");
		//printf("NT : Deleted %s \n",mVariable[index].name);			
		strcpy(mVariable[index].name," ");			
		mVariable[index].value = -1;
		mVariable[index].monitorID = -1;
		sprintf(ack, "%d", 1);			//Preparing ack to send to client, -1 indicates error
		outMailHdr.length = strlen(ack) + 1;		//storing the length of ack data in Mail Header
		*tmonitorCount=monitorCount;
						
		if(clientMID==postOffice->GetMachineID() && clientMBN==currentThread->mailBoxNo)
		{
			success = reliablePost->Send(outPktHdr, outMailHdr, ack); //sending the packet containing ack to client
			//printf("NT : Server Sending the Response to the Client\n");
			if ( !success ) 			//check to see if packet is sent
			{
				printf("NT : The postOffice Send failed.\n");
				interrupt->Halt();
			}
		}
	}
}
		

		
		
		
		
		


void Server()
{

	int lockCount = 0;			//count to check the number of locks created
	int cvCount = 0;			//count to check the number of CV created
	int monitorCount = 0;		//Count to check the number of monitor variables created
	int semaphoreCount = 0; 	//Count to check the number of semaphore variables created
	int listCount = 0;			//count to check the number of List created
	listTablel slt[MAX_VARS_List];
	ServerLock sLocks[MAX_VARS_LC_CV]; 		//lock structure objects
	ServerConditionVariable sCV[MAX_VARS_LC_CV];		//number of objests for CV structure
	Monitor mVariable[MAX_VARS_MV];	//number of objests for CV structure

	char clients[ClientCharSize];
	int clientsLen=0,noOfClients;
	char c;

    PacketHeader outPktHdr, inPktHdr;			//declaring objects of PacketHeader
    MailHeader outMailHdr, inMailHdr;		//declaring objects of MailHeader

    char buffer[MaxMailSize],buffer2[MaxMailSize], *param[8];
    bool success;
    int requestType, clientMID, clientMBN, rv;
    char *ack=" ";
	char *ctemp;
	int itemp1,itemp2;

	long int litemp;
	int64_t tempi64;
		
	struct MyMessages *msgtemp;

	bool doit;


	
//	printf("NT : inside server\n");


    outPktHdr.to = 0;					//Hardcoding the server machine ID
    outMailHdr.to = 0;
    outMailHdr.from = currentThread->mailBoxNo;
	bzero(buffer,MaxMailSize);
	strcpy(buffer, "MyInfo"); 		//Preparing ack to send to client, -1 indicates error
	buffer[6]='\0';
	outMailHdr.length = strlen(buffer)+1;		//storing the length of ack data in Mail Header
	
	
	success = reliablePost->Send(outPktHdr, outMailHdr, buffer); 	//sending the packet containing ack to client
	//	printf("NT : data sent to reg server\n");
	if ( !success ) 		//check to see if packet is sent
	{
		printf("NT : The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}			


	while(1)
	{
		
		reliablePost->Receive(currentThread->mailBoxNo, &inPktHdr, &inMailHdr, buffer);			

		if(!(inMailHdr.from==0 && inPktHdr.from==0))
			continue;
		else
			break;
	}
	//	printf("NT : NTmsg22\"%s\" from %d, box %d to %d,box %d\n", buffer, inPktHdr.from, inMailHdr.from,inPktHdr.to,inMailHdr.to);
			
	strcat(clients,buffer);

	ctemp=strtok(buffer, "#");
	itemp1=atoi(ctemp);

	noOfClients=itemp1;
	while(1)
	{
		int j=0;
		for(int i=0;i<strlen(clients);i++)
		{
			if(clients[i]=='#')
				j++;
		}

		if(j>itemp1)
			break;


		while(1)
		{
			bzero(buffer,MaxMailSize);
			reliablePost->Receive(currentThread->mailBoxNo, &inPktHdr, &inMailHdr, buffer);			
			if(!(inMailHdr.from==0 && inPktHdr.from==0))
				continue;
			else
				break;
				
		}
		//	printf("NT : NTmsg22\"%s\" from %d, box %d to %d,box %d\n", buffer, inPktHdr.from, inMailHdr.from,inPktHdr.to,inMailHdr.to);

		strcat(clients,buffer);
	}
	
	
	bzero(buffer,MaxMailSize);
	strcpy(buffer, "GotAll"); 		//Preparing ack to send to client, -1 indicates error
	buffer[6]='\0';
	outMailHdr.length = strlen(buffer)+1;		//storing the length of ack data in Mail Header
	
	
	success = reliablePost->Send(outPktHdr, outMailHdr, buffer); 	//sending the packet containing ack to client
	//	printf("NT : data sent to reg server\n");
	if ( !success ) 		//check to see if packet is sent
	{
		printf("NT : The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}			


	
	


	struct MyMessages *bufMsg;
	List *bufMsgList;
	bufMsgList=new List;
	
	

	struct State *states;

	states=new struct State[noOfClients];

	clientsLen=strlen(clients);

	//printf("NT : %s %d\n",clients,clientsLen);

	ctemp=NULL;
	ctemp= strtok(clients, "$#");
	itemp1=0;
   	while (ctemp != NULL) 
   	{
   		ctemp= strtok(NULL, "$#");
		if(ctemp==NULL)
			break;
		states[itemp1].mid= atoi(ctemp);

   		ctemp= strtok(NULL, "$#");
		states[itemp1].mbn= atoi(ctemp);
		states[itemp1].timeStamp= 0;
		itemp1++;
   	}

	

	printf("NT : \nSERVER STARTING mbn %d\n",currentThread->mailBoxNo);

	struct timeval tv;
	long int timeStamp=0;
	char ttime[17];

	while(true)
	{
		itemp1=0;
		ctemp=NULL;
//		printf("NT : Server receiving\n");

        reliablePost->Receive(currentThread->mailBoxNo, &inPktHdr, &inMailHdr, buffer);

		clientMID = inPktHdr.from;
		clientMBN = inMailHdr.from;
		itemp2=inMailHdr.length;

        
		fflush(stdout);


//if message from user thread
		if((clientMID== postOffice->GetMachineID() )&& (clientMBN==(currentThread->mailBoxNo+1)))
		{			
		//printf("NT : umsg\"%s\" from %d, box %d to %d,box %d\n", buffer, inPktHdr.from, inMailHdr.from,inPktHdr.to,inMailHdr.to);
			gettimeofday(&tv, NULL);
			sprintf(ttime,"%11d%6d", tv.tv_sec, tv.tv_usec);
	//		printf("NT : %s\n",ttime);
			for(int i=0;i<17;i++)
				if(ttime[i]==' ')
					ttime[i]='0';
	//		printf("NT : %s\n",ttime+6);

			addTimeStamp(ttime+6,buffer,itemp2);
			//SendToAll(clients,clientsLen,buffer,itemp2+11);
			for(int i=0; i<noOfClients; i++)
			{
				outPktHdr.to = states[i].mid;					
			    outMailHdr.to =states[i].mbn;
				clientMID= states[i].mid;					
			    clientMBN=states[i].mbn;
				
		    	outMailHdr.from = currentThread->mailBoxNo;
				outMailHdr.length = strlen(buffer)+1;		//storing the length of ack data in Mail Header
				success = reliablePost->Send(outPktHdr, outMailHdr, buffer); 	//sending the packet containing ack to client
		//		printf("NT : data sent to server mid %d box %d ,msg %s\n",clientMID,clientMBN,buffer);
				if ( !success ) 		//check to see if packet is sent
				{
					printf("NT : The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
					interrupt->Halt();
				}				
			}		

			
		}
		//if message from NT thread
		else
		{
	//		printf("NT : NTmsg\"%s\" from %d, box %d to %d,box %d\n", buffer, inPktHdr.from, inMailHdr.from,inPktHdr.to,inMailHdr.to);
			ctemp=NULL;
			bzero(buffer2,MaxMailSize);
			strcpy(buffer2,buffer);
				
			ctemp=strtok(buffer,"#");
			timeStamp=atol(ctemp);
	//		printf("NT : %ld\n",timeStamp);
			ctemp=strtok(NULL,"$");
//			itemp2=-1;
			itemp2=atoi(ctemp);

			
			if(itemp2==20)
			{
			
			printf("NT : sending the ack back\n",timeStamp);
//*********************************************when needs to waiting for other threads code*********************************
									outPktHdr.to = postOffice->GetMachineID();					
	outMailHdr.to = currentThread->mailBoxNo+1;
	outMailHdr.from = currentThread->mailBoxNo;
	strcpy(buffer,"111");
	outMailHdr.length = strlen(buffer)+1;	//storing the length of ack data in Mail Header
	
	success = reliablePost->Send(outPktHdr, outMailHdr, buffer);	//sending the packet containing ack to client
//	printf("NT : data sent other user mbn %d\n",currentThread->mailBoxNo);
	if ( !success ) 		//check to see if packet is sent
	{
		printf("NT : The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}		
continue;
}
			
			if(itemp2!= 25)
			{
				bufMsg=new MyMessages;
				bufMsg->mid=inPktHdr.from;
				bufMsg->mbn=inMailHdr.from;
				bzero(bufMsg->msg,MaxMailSize);
				strcpy(bufMsg->msg,buffer2);
				bufMsg->timeStamp=timeStamp;
	
				for(int i=0;i<noOfClients;i++)
					if(states[i].mid==inPktHdr.from && states[i].mbn==inMailHdr.from)
						if(states[i].timeStamp<timeStamp)
							states[i].timeStamp=timeStamp;
				
				tempi64=timeStamp*1000+inPktHdr.from*100+inMailHdr.from;
				bufMsgList->SortedInsert((void *)bufMsg,timeStamp);


				gettimeofday(&tv, NULL);
				sprintf(ttime,"%11d%6d", tv.tv_sec, tv.tv_usec);
		//		printf("NT : %s\n",ttime);
				for(int i=0;i<17;i++)
					if(ttime[i]==' ')
						ttime[i]='0';
				
		//		printf("NT : %s\n",ttime+6);
				bzero(buffer,MaxMailSize);
			
				strcpy(buffer,"25$");
				itemp2=3;
				addTimeStamp(ttime+6,buffer,itemp2);
				//SendToAll(clients,clientsLen,buffer,itemp2+11);
				for(int i=0; i<noOfClients; i++)
				{
					outPktHdr.to = states[i].mid;					
					outMailHdr.to =states[i].mbn;
					clientMID= states[i].mid;					
					clientMBN=states[i].mbn;
					
					outMailHdr.from = currentThread->mailBoxNo;
					outMailHdr.length = strlen(buffer)+1;		//storing the length of ack data in Mail Header
					success = reliablePost->Send(outPktHdr, outMailHdr, buffer); 	//sending the packet containing ack to client
		//			printf("NT : data sent to server mid %d box %d ,msg %s\n",clientMID,clientMBN,buffer);
					if ( !success ) 		//check to see if packet is sent
					{
						printf("NT : The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
						interrupt->Halt();
					}				
				}

			
			}
		/*	else
			{//update timestamps*/

				for(int i=0;i<noOfClients;i++)
					if(states[i].mid==inPktHdr.from && states[i].mbn==inMailHdr.from)
						if(states[i].timeStamp<timeStamp)
							states[i].timeStamp=timeStamp;
			/*}*/



			while(1)
				{
					litemp=0;
					msgtemp=NULL;
					doit=TRUE;
					msgtemp=(struct MyMessages *)bufMsgList->SortedRemove(&tempi64);
					litemp=tempi64/100;
					if(msgtemp)
					{
						for(int i=0;i<noOfClients;i++)
							if(states[i].timeStamp<litemp)
							{
								doit=FALSE;
								bufMsgList->SortedInsert((void *)msgtemp,tempi64);
								break;
							}
				
					}
					else
						break;

					if(doit)
					{
					//	printf("NT : \nmy %d %dprocessing message %d %d %s %ld\n",postOffice->GetMachineID(),currentThread->mailBoxNo,msgtemp->mid,msgtemp->mbn,msgtemp->msg,msgtemp->timeStamp);

					//for(int i=0;i<noOfClients;i++)
							//printf("NT : %d %d %d \t",states[i].mid,states[i].mbn,states[i].timeStamp);
						//printf("NT : \n\n");


						ctemp= strtok(msgtemp->msg, "#$");

						ctemp= strtok(NULL, "#$");
						itemp1=0;
						while (ctemp != NULL) 
						{
							param[itemp1] = ctemp;
							itemp1++;
							ctemp= strtok(NULL, "$");
						}
						
						requestType = atoi(param[0]);
						
						
						
						
						
						ctemp=NULL;
						
						
						switch (requestType) 
						{
							case 1: 				// SC_CREATELOCK		1. request 2. name
								//printf("NT : IN CreateLock.\n");
								fflush(stdout);
								ctemp=param[1]; 
								Server_CreateLock(ctemp, msgtemp->mid,msgtemp->mbn,sLocks,&lockCount);
								
								break;
						
							case 2: 		// Destroy lock 
								//printf("NT : IN DestroyLock.\n");
								fflush(stdout);
								int reqLock;
								reqLock = atoi(param[1]);
								Server_DestroyLock(reqLock, msgtemp->mid,msgtemp->mbn,sLocks,&lockCount);
								break;
							case 3: 	// Server Lock acquire
							
								//printf("NT : IN Acquire.\n");
							
								reqLock = atoi(param[1]);
								
								Server_Acquire(reqLock, msgtemp->mid,msgtemp->mbn,sLocks,&lockCount);
							
								break ;
						
							case 4: 	// Server Lock release
							
								//printf("NT : IN Release.\n");	//	reqLock;	
							
								reqLock = atoi(param[1]);
					
								Server_Release(reqLock, msgtemp->mid,msgtemp->mbn, 1,sLocks,&lockCount); 
						
					
								break ; 
				
							case 5: 				// SC_CREATECV		1. request 2. name
							
								//printf("NT : CreateCV Request.\n");
								fflush(stdout);
								char *str1=param[1];	
								Server_CreateCondition(str1, msgtemp->mid,msgtemp->mbn,sCV,&cvCount);
						
								break ;
					
							case 6: 		// Destroy lock 
							
								//printf("NT : IN DestroyCV.\n");
								
								fflush(stdout);
							
								int reqCV;
								reqCV	= atoi(param[1]);
							
								Server_DestroyCV(reqCV, msgtemp->mid,msgtemp->mbn,sCV,&cvCount);
							
					   
								break;	
					
							case 7: 		// Wait Condition 
							
								//printf("NT : IN waitCV.\n");
							
								fflush(stdout);
							
								int reqCV1, reqLock1;
								reqCV1	= atoi(param[1]);
								reqLock1	= atoi(param[2]);

								
								rv=Server_WaitCV(reqCV1,reqLock1,msgtemp->mid,msgtemp->mbn,sLocks,&lockCount,sCV,&cvCount);
								//printf("NT : %d\n",rv);	
								if(rv == -1)
								{
									sprintf(ack, "%d", -1); 		//Preparing ack to send to client, -1 indicates error
									outMailHdr.length = strlen(ack) + 1;		//storing the length of ack data in Mail Header

									if(msgtemp->mid==postOffice->GetMachineID() && msgtemp->mbn==currentThread->mailBoxNo)
									{
									
										outPktHdr.to = msgtemp->mid;			//storing the Machine ID of client to whom this packet is to send
									    outMailHdr.to = msgtemp->mbn+1;			//storing the MailBox number of client to whom this packet is to send
										outMailHdr.from=currentThread->mailBoxNo;
										success = reliablePost->Send(outPktHdr, outMailHdr, ack); 	//sending the packet containing ack to client
										//printf("NT : Server Sending the Response to the Client\n");
										if ( !success ) 			//check to see if packet is sent
										{
											printf("NT : The postOffice Send failed.\n");
											interrupt->Halt();
										}
									}

								}
							
								break;	
					
							case 8: 		// Signal Codition 
							
								//printf("NT : IN SignalCV.\n");
							
								fflush(stdout);
							
								int reqCV2, reqLock2;
								reqCV2	= atoi(param[1]);
								reqLock2 = atoi(param[2]);
							
								rv = Server_SignalCV(reqCV2,reqLock2,msgtemp->mid,msgtemp->mbn,sLocks,&lockCount,sCV,&cvCount);
								
								if(rv==-1)
								{
									sprintf(ack, "%d", -1); 	//Preparing ack to send to client, -1 indicates error
								}
								else
								{
									sprintf(ack, "%d", 1);
								}

								
								outMailHdr.length = strlen(ack) + 1;		//storing the length of ack data in Mail Header
								
								if(msgtemp->mid==postOffice->GetMachineID() && msgtemp->mbn==currentThread->mailBoxNo)
								{
									outPktHdr.to = msgtemp->mid;			//storing the Machine ID of client to whom this packet is to send
									outMailHdr.to = msgtemp->mbn+1; 		//storing the MailBox number of client to whom this packet is to send
									outMailHdr.from=currentThread->mailBoxNo;
									success = reliablePost->Send(outPktHdr, outMailHdr, ack); 	//sending the packet containing ack to client
									//printf("NT : Server Sending the Response to the Client\n");
									if ( !success ) 			//check to see if packet is sent
									{
										printf("NT : The postOffice Send failed.\n");
										interrupt->Halt();
									}
								}
							   
								break;	
					
							case 9: 		// Broadcast Condition
						
								//printf("NT : IN BroadcastCV.\n");
							
								fflush(stdout);
							
								int reqCV3, reqLock3;
								reqCV3 = atoi(param[1]);
								reqLock3 = atoi(param[2]);
									
								rv=Server_BroadcastCV(reqCV3, reqLock3,msgtemp->mid,msgtemp->mbn,sLocks,&lockCount,sCV,&cvCount);

								if(rv==-1)
								{
									sprintf(ack, "%d", -1); 	//Preparing ack to send to client, -1 indicates error
								}
								else
								{
									sprintf(ack, "%d", 1);
								}

								
								outMailHdr.length = strlen(ack) + 1;		//storing the length of ack data in Mail Header
								
								if(msgtemp->mid==postOffice->GetMachineID() && msgtemp->mbn==currentThread->mailBoxNo)
								{
									outPktHdr.to = msgtemp->mid;			//storing the Machine ID of client to whom this packet is to send
									outMailHdr.to = msgtemp->mbn+1; 		//storing the MailBox number of client to whom this packet is to send
									outMailHdr.from=currentThread->mailBoxNo;
									success = reliablePost->Send(outPktHdr, outMailHdr, ack); 	//sending the packet containing ack to client
									//printf("NT : Server Sending the Response to the Client\n");
									if ( !success ) 			//check to see if packet is sent
									{
										printf("NT : The postOffice Send failed.\n");
										interrupt->Halt();
									}
								}
							   	
					   
								break;	
						
							case 10:			//Create Monitor	
						
								//printf("NT : In Creating Monitor Variable\n");
								
								fflush(stdout);
						
								char *str4 = param[1];		//storing the name parameter
						
								int length;
						
								length = atoi(param[2]);	//storing the length parameter
								int mValue2; 				//value to set for monitor variable
						
								mValue2 = atoi(param[3]);	//storing the value passes by thr client
						
						
								Server_CreateMonitor(str4, msgtemp->mid, msgtemp->mbn, length,mVariable,&monitorCount,mValue2);	
						
								break;	
						
							case 11:			//Set monitor values
					
								//printf("NT : In Set Monitor variable value\n");
						
								fflush(stdout);
							
								int index = atoi(param[1]); 	//storing the index passed by client
						
								int mValue; 				//value to set for monitor variable
						
								mValue = atoi(param[2]);	//storing the value passes by thr client
						
								Server_SetMonitor(index, msgtemp->mid, msgtemp->mbn,mValue,mVariable,&monitorCount);   
						
								break;	
								
							case 12:			//Get monitor value
					
								//printf("NT : In Get Monitor Variable value\n");
						
								fflush(stdout);
						
								int index2 = atoi(param[1]);		//storing the index passed by client
						
								Server_GetMonitor(index2,msgtemp->mid, msgtemp->mbn,mVariable,&monitorCount);   
						
								break;	
						
							case 13:			//destroy monitor
					
								//printf("NT : In Destroy Monitor\n");
							
								fflush(stdout);
						
								int index3 = atoi(param[1]);		//storing the index passed by client
						
								Server_DestroyMonitor(index3, msgtemp->mid, msgtemp->mbn,mVariable,&monitorCount);   
						
								break;	
						
								
							default:
							
								//printf("NT : WRONG REQUEST.\n");
								break;
					
						}	
					}
					else
						break;

				}
					
		}

	}
}


char * addTimeStamp(char *timeStamp, char *msg, int len)
{

	char tempMsg[MaxMailSize];
	bzero(tempMsg,MaxMailSize);

	strncpy(tempMsg,msg,len);
	

	bzero(msg,MaxMailSize);
	strncpy(msg,timeStamp,9);
	msg[9]='#';
	
	strcat(msg,tempMsg);
	return msg;
	
}

void SendToAll(char *clients,int clientsLen, char *msg, int msgLen)
{

    PacketHeader outPktHdr;			//declaring objects of PacketHeader
    MailHeader outMailHdr;		//declaring objects of MailHeader

    char buffer[MaxMailSize];
    int clientMID, clientMBN;
	bool success; 
	
	char *ctemp,*ctemp2;
	int itemp;
	int noOfClients;

	ctemp2=new char[clientsLen+1];
	bzero(ctemp2,clientsLen+1);
	strcpy(ctemp2,clients);
	ctemp2[clientsLen]='\0';
	//printf("NT : in sent2as  %s\n",ctemp2);
	ctemp=NULL;	
	ctemp=strtok(ctemp2, "#");
	noOfClients=atoi(ctemp);


	//printf("NT : in sent2as   %d %s msg %s\n",noOfClients,ctemp2,msg);
	for(int i=0; i<noOfClients;i++)
	{
		ctemp=strtok(NULL, "$#");
		if(ctemp==NULL)
			break;
		clientMID=atoi(ctemp);
		ctemp=strtok(NULL, "$#");
		clientMBN=atoi(ctemp);		
		outPktHdr.to = clientMID;					
	    outMailHdr.to = clientMBN;
    	outMailHdr.from = currentThread->mailBoxNo;
		outMailHdr.length = msgLen +1;		//storing the length of ack data in Mail Header
		success = reliablePost->Send(outPktHdr, outMailHdr, msg); 	//sending the packet containing ack to client
		//printf("NT : data sent to server mid %d box %d ,msg %s\n",clientMID,clientMBN,msg);
		if ( !success ) 		//check to see if packet is sent
		{
			printf("NT : The postOffice Send failed. You must not have the server Nachos running. Terminating Nachos.\n");
			interrupt->Halt();
		}			
	}
	
}

void GroupRegServer(int n)
{

	char clients[ClientCharSize];	//string to be send as reply
	char ctemp2[10];
	int clientsLen=0;
	int temp =0;
	char ctemp3[ClientCharSize];
	char *ctemp;
	PacketHeader outPktHdr,inPktHdr; 		//declaring objects of PacketHeader
	MailHeader outMailHdr,inMailHdr;		//declaring objects of MailHeader
    int clientMID, clientMBN;
	char buffer[MaxMailSize];

	bool success;
	currentThread->mailBoxNo=0;

	bzero(clients,ClientCharSize);
	bzero(buffer,MaxMailSize);
	//printf("NT : %d\n",n);
	sprintf (clients, "%d#",n);
	//printf("NT : %s\n",clients);
	clientsLen=strlen(clients);



	while(temp <n)
	{
		int z=0;
	
		printf("NT : Group Reg Server Waiting for Client's request\n");

		reliablePost->Receive(0, &inPktHdr, &inMailHdr, buffer);

		clientMID = inPktHdr.from;
		clientMBN = inMailHdr.from;
	
		printf("NT : Got \"%s\" from %d, box %d\n", buffer, inPktHdr.from, inMailHdr.from);	//-------------------------
	
		fflush(stdout);

		if(! strcmp(buffer,"MyInfo")) 
		{
			//printf("NT : gotit\n");
			sprintf(ctemp2, "%d$%d#", clientMID, clientMBN);
			strcat(clients,ctemp2);
			//printf("NT : gotit2\n");

			temp++;
		}

	}

	//printf("NT : gotit3\n");
	//printf("NT : %s %d %s %d\n",clients,strlen(clients),clients,strlen(clients));
	//printf("NT : gotit4\n");

	printf("NT : %s %d \n",clients,strlen(clients));
	strcpy(ctemp3,clients);
	//printf("NT : gotit5\n");
	ctemp=ctemp3;
	if(strlen(ctemp)>MaxMailSize-1)
		temp=MaxMailSize-1;
	else
		temp=strlen(ctemp);
		//printf("NT : gotit6\n");
		
	while(temp>0)
	{
		strncpy(buffer,ctemp,temp);
		//printf("NT : gotit7\n");
		buffer[temp]='\0';
		
		//printf("NT : gotit %s\n",buffer);
		SendToAll(clients,strlen(clients),buffer,strlen(buffer));
			//printf("NT : gotit8\n");
		ctemp+=temp;
		if(strlen(ctemp)>MaxMailSize-1)
			temp=MaxMailSize-1;
		else
			temp=strlen(ctemp);
	}
	
	
	temp=0;
	while(temp <n)
	{
		
		bzero(buffer,MaxMailSize);
		printf("NT : Group Reg Server Waiting for Client's reply\n");

		reliablePost->Receive(0, &inPktHdr, &inMailHdr, buffer);

		clientMID = inPktHdr.from;
		clientMBN = inMailHdr.from;
	
		printf("NT : Got \"%s\" from %d, box %d\n", buffer, inPktHdr.from, inMailHdr.from);	//-------------------------
	
		fflush(stdout);

		if(! strcmp(buffer,"GotAll")) 
		{
			temp++;
		}

	}
	
	printf("NT : %s %d \n",clients,strlen(clients));
	bzero(buffer,MaxMailSize);
	strcpy(buffer,"20#20$");
	SendToAll(clients,strlen(clients),buffer,strlen(buffer));

	
}	

void MailTest(int qq)
{

}
