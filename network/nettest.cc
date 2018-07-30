// nettest.cc 
//	Test out message delivery between two "Nachos" machines,
//	using the Post Office to coordinate delivery.
//
//	Two caveats:
//	  1. Two copies of Nachos must be running, with machine ID's 0 and 1:
//		./nachos -m 0 -o 1 &
//		./nachos -m 1 -o 0 &
//
//	  2. You need an implementation of condition variables,
//	     which is *not* provided as part of the baseline threads 
//	     implementation.  The Post Office won't work without
//	     a correct implementation of condition variables.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"


void MailTest(int farAddr)
{
	// Message Format -> Seq No | Type of Message | Timestamp | Request Type | Request's Info
	printf("Started server\n"); fflush(stdout);
	
	int sequenceNo=0;			// For the sequence no. in the msg
	int typeOfMsg=0;			// Type of message = 0 for the Group Registration Message			
	int requestType=-1;			// -1 as NA here
	time_t ltime;				// For the timestamp


	// Message Preperation ends here	

	int noOfUserThreads, i;
		
	struct serverDB grpServerDB[MAX_THREADS_PER_PROCESS];		// Project 4 - Used to store [machine ID, mailbox NO);

    	PacketHeader outPktHdr, inPktHdr;
    	MailHeader outMailHdr, inMailHdr;
    	char *data = "";
		char *data1="";


   
    	char buffer[MaxMailSize];
	
	// Get the number of user threads
	
	printf("\nEnter the number of User Threads: "); fflush(stdout);
	scanf("%d", &noOfUserThreads);

	i=noOfUserThreads - 1;						// Index in structure goes from 0 to n-1
	
	// Accept information from noOfuserThreads number of Threads

	while(i >= 0)
	{	printf("Reached loop\n"); fflush(stdout);
		// Wait for the first message from the other machine
    		postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
    									//fflush(stdout) reqd???????	
	printf("Recvd sth\n");  fflush(stdout);
		// Stored both IDs for the networking thread
		grpServerDB[i].machineID=inPktHdr.from;			//Got Machine ID !!
		grpServerDB[i].mailboxNO=inMailHdr.from;		//Got Mailbox NO !!

		
		// Reply with noUserThreads so that each networking thread can receive for that
		// many number of times

		// Time Stamp Generation Part
		ltime=time(NULL);	

		typeOfMsg=0;
		data = "";
		
		printf("seq, type, reqtype, number of threads -> %d %d %d %d\n", sequenceNo, typeOfMsg,  requestType,noOfUserThreads); fflush(stdout);
		
//		sprintf(data, "%d@%d@%s@%d@%d", sequenceNo++, typeOfMsg, asctime(localtime(&ltime)), requestType, noOfUserThreads);
		sprintf(data, "%d@%d@%d@%d", sequenceNo++, typeOfMsg,  requestType, noOfUserThreads);

      	 	// construct packet, mail header for original message
		// To: destination machine, mailbox 0
		// From: our machine, reply to: mailbox 1
    		outPktHdr.to = inPktHdr.from;		
    		outMailHdr.to = inMailHdr.from;
    		outMailHdr.from = 1;
    		outMailHdr.length = strlen(data) + 1;
			printf("just before send %s\n", data); fflush(stdout);
    		// Send the first message
    		bool success = postOffice->Send(outPktHdr, outMailHdr, data); 
			printf("just after send\n"); fflush(stdout);
    		if ( !success ) 
		{
      			printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n"); fflush(stdout);
      			interrupt->Halt();
    		}


		i--;					//For next iteration
	}

	printf("I have db now man\n"); fflush(stdout);

	// By now all the networking threads have their Machine IDs and Mailbox NOs stored at the group registration server
	// Server replies with the number of userthreads
	// Now broadcasting this information to all the networking threads

	
	for(int xyz=0; xyz < noOfUserThreads; xyz++)
		printf("Machine ID: %d\tMailbox No.: %d\n", grpServerDB[xyz].machineID, grpServerDB[xyz].mailboxNO); fflush(stdout);

	
	
	int outMachineID, outMailboxNO;
	int j, counter;

	i=noOfUserThreads - 1;

	//Broadcasting Database
	
	while (i >= 0)
	{
		outMachineID=grpServerDB[i].machineID;
		outMailboxNO=grpServerDB[i].mailboxNO;		
		
		typeOfMsg=1;					// Type of message = 1 for the Group Registration Broadcast Message
	
		
		int innerCounter=0;

		char *newData = "";
		
		while(innerCounter < noOfUserThreads)
		{
			printf("Entered loop again\n");	fflush(stdout);
		

			sprintf(newData, "%d@%d@%d@%d", sequenceNo++, typeOfMsg, grpServerDB[innerCounter].machineID, grpServerDB[innerCounter++].mailboxNO);
	
		
	
	
			
		 	// construct packet, mail header for original message
			// To: destination machine, mailbox 0
			// From: our machine, reply to: mailbox 1
    			outPktHdr.to = outMachineID;		
    			outMailHdr.to = outMailboxNO;
    			outMailHdr.from = 1;	
    			outMailHdr.length = strlen(newData) + 1;
	
//				printf("This send killed me %d %d %s\n", j, outMailHdr.length); fflush(stdout);
	
    			// Send the first message
					printf("Data -> %s\n", newData);	fflush(stdout);

					
				bool success = postOffice->Send(outPktHdr, outMailHdr, newData); 
	
    			if ( !success ) 
				{
      				printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      				interrupt->Halt();
    			}
				
				printf("Data send\n");	fflush(stdout);
		}
	
		i--;
	}
	
	
	//Broadcasting Database
/*
	while (i >= 0)
	{
		outMachineID=grpServerDB[i].machineID;
		outMailboxNO=grpServerDB[i].mailboxNO;		
		
		typeOfMsg=1;					// Type of message = 1 for the Group Registration Broadcast Message
		

		printf("main while loop\n");

		counter=0;

		while ( no_Per_Thread <= (noOfUserThreads - counter)) 
		{			printf("while loop 2\n"); fflush(stdout);
				char *data = "";
				sprintf(data, "%d@%d", sequenceNo++, typeOfMsg);
	
				printf("Data -> %s\n", data);
	
			j=0;
			while(j < no_Per_Thread)
			{	printf("while loop 3\n"); fflush(stdout);
				outMachineID=grpServerDB[counter].machineID;
				outMailboxNO=grpServerDB[counter++].mailboxNO;		

				char *data1 = "";
				sprintf(data1, "@%d@%d", outMachineID, outMailboxNO);
				strcat(data, data1);

				printf("Data -> %s\n", data);

				
				j++;
			}

			printf("You reached here, WOW, impressive\n");fflush(stdout);
			
		 	// construct packet, mail header for original message
			// To: destination machine, mailbox 0
			// From: our machine, reply to: mailbox 1
    			outPktHdr.to = outMachineID;		
    			outMailHdr.to = outMailboxNO;
    			outMailHdr.from = 1;	//printf("jo bhi\n");
    			outMailHdr.length = strlen(data) + 1;	//printf("jo bhi sequel\n");
	
//				printf("This send killed me %d %d %s\n", j, outMailHdr.length); fflush(stdout);
	
    			// Send the first message
    			bool success = postOffice->Send(outPktHdr, outMailHdr, data); 
	
    			if ( !success ) 
				{
      				printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      				interrupt->Halt();
    			}
		}

		int endValue=1000;      	
		j=0;
		int initiate=0;
		
	if(j < noOfUserThreads % no_Per_Thread)		
	{	
			
			data = "";
			
			printf("\n\nWorking on thread \n");	
			fflush(stdout);			
						
	//		sprintf(data, "%d@%d", sequenceNo++, typeOfMsg);
	//		printf("Data with seqno and type -> %s\n", data);

	
			
		while(j < noOfUserThreads % no_Per_Thread)
		{	
			if (initiate == 0)
			{	printf("******************************************************************\n");fflush(stdout);
				sprintf(data, "%d@%d", sequenceNo++, typeOfMsg);
				initiate++;
			}
			
			char *data1 = "";
			
			printf("I sent-> %d %d %d\n", grpServerDB[counter].machineID, grpServerDB[counter].mailboxNO, counter );
			
			sprintf(data1, "@%d@%d", grpServerDB[counter].machineID, grpServerDB[counter].mailboxNO);
	
			printf("Sent before uff\n");
	
			strcat(data, data1);
			
			printf("Data -> %s\n", data);
			
			printf("Sent uff\n");
			
			counter++;
			j++;
		}
	}

		char *data1 = "";
		sprintf(data1, "@%d", endValue);
		strcat(data, data1);

		printf("Data -> %s\n", data);
		
		 // construct packet, mail header for original message
		// To: destination machine, maixlbox 0
		// From: our machine, reply to: mailbox 1
    		outPktHdr.to = outMachineID;		
    		outMailHdr.to = outMailboxNO;
    		outMailHdr.from = 1;
    		outMailHdr.length = strlen(data) + 1;
	
			printf("this 1 crashed man! %d %d %s leng %d max %d", outPktHdr.to, outMailHdr.to, data, outMailHdr.length, MaxMailSize);  fflush(stdout);
	
	
    		// Send the first message
    		bool success = postOffice->Send(outPktHdr, outMailHdr, data); 
	
    		if ( !success ) 
		{
      			printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      			interrupt->Halt();
    		}

			printf("Crashed sequel\n");

		i--;					//For next iteration
	}
*/
	
	

	// Must receive the "db_received" ack from each networking thread



	// Then we're done!
	   interrupt->Halt();
}
