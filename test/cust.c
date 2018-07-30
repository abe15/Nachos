#include "syscall.h"



enum orderTakerStatus {OTFree, OTBusy, OTWaiting};	
enum tableStatus {TFree, TBusy};
enum waiterStatus {WFree, WBusy};	
/*Structure for the Customer*/
typedef struct Cust	
{
/*
  	CreateMV("custID",6);		/*customer id for its identification
 	CreateMV("myOrderTaker",12);	/*denotes the index of the OrderTaker assigned to the customer
	CreateMV("custTokenNo",11);	/*denotes the Token No. given to the Customer by the OrderTaker
	CreateMV("orderType",9);		/*orderType denotes the type of Customer's Order 0 for Eat In & 1 for To Go 
	
	CreateMV("toGoWaiting",11);	/*denotes if the Customer is waiting in the To Go Monitor for the tokens to be 
				/*matched by the bagger. Initialized to 0 if not waiting, else 1
	
	CreateMV("custDoA",7);		/*denotes if the Customer is Dead or Alive
				/*Initialized to 0 if Customer's Dead, 1 if Customer's Alive

	CreateMV("custTableID",11);	/*initialized to -1
	CreateMV("waiterAssigned",14);	/*initialized to -1


	/*Food items available at the Restaurant. Set to the quantity of items ordered,if ordered else 0

	CreateMV("sixBurger",9);		/* 6$ Burger
	CreateMV("threeBurger",11);	/* 3$ Burger
	CreateMV("veggieBurger",12);	/* Veggie Burger
	CreateMV("frenchFries",11);	/* French Fries
	CreateMV("soda",4);		/* Soda
	
*/

int custID;		/*customer id for its identification*/
 	int myOrderTaker;	/*denotes the index of the OrderTaker assigned to the customer*/
	int custTokenNo;	/*denotes the Token No. given to the Customer by the OrderTaker*/
	int orderType;		/*orderType denotes the type of Customer's Order 0 for Eat In & 1 for To Go*/ 
	
	int toGoWaiting;	/*denotes if the Customer is waiting in the To Go Monitor for the tokens to be*/ 
				/*matched by the bagger. Initialized to 0 if not waiting, else 1*/
	
	int custDoA;		/*denotes if the Customer is Dead or Alive*/
				/*Initialized to 0 if Customer's Dead, 1 if Customer's Alive*/

	int custTableID;	/*initialized to -1*/
	int waiterAssigned;	/*initialized to -1*/


	/*Food items available at the Restaurant. Set to the quantity of items ordered,if ordered else 0*/

	int sixBurger;		/* 6$ Burger*/
	int threeBurger;	/* 3$ Burger*/
	int veggieBurger;	/* Veggie Burger*/
	int frenchFries;	/* French Fries*/
	int soda;		/* Soda*/

}Cust;




typedef struct OrderTaker
{
/*
	CreateMV("otID",4);		/*OrderTaker id for its identification
	CreateMV("otTokenNo",9);		/*OrderTaker stores the Token No. assigned to the Customer

	CreateMV("otBagTokenNo",12);	/*When the OrderTaker acts as a bagger, the Token No he bags is stored here
				/*Value initialized to -1

	CreateMV("otCash",6);		/*Denotes the amount of cash available with the OrderTaker
				/*initialized to 0.00

	CreateMV("otStatusMV",10);	/*Denotes if the OrderTaker is Free, Busy or Waiting
	*/
	
	int otID;		/*OrderTaker id for its identification*/
	int otTokenNo;		/*OrderTaker stores the Token No. assigned to the Customer*/

	int otBagTokenNo;	/*When the OrderTaker acts as a bagger, the Token No he bags is stored here*/
				/*Value initialized to -1*/

	int otCash;		/*Denotes the amount of cash available with the OrderTaker*/
				/*initialized to 0.00*/
				
	int otStatusMV;	/*Denotes if the OrderTaker is Free, Busy or Waiting*/ 
}OrderTaker;



typedef struct Waiter
{	/*
	CreateMV("wID",3);
	CreateMV("wTokenNo",8);
	CreateMV("wStatus",7);
	*/
	
	int wID;
	int wTokenNo;
	int wStatus;
	
	
}Waiter;



typedef struct T1
{/*
	CreateMV("tID",3);
	CreateMV("tStatus",7);
*/

int tID;
int tStatus;
}T1;



typedef struct Manager
{
/*	CreateMV("mID",3);
	CreateMV("mCash",5);
	
	*/
int mID;
	int mCash;	
	
}Manager;



typedef struct Cook
{
/*
	CreateMV("cookid",6);
	CreateMV("Status",6);
	*/
	int cookid;
	int Status;
}Cook;
#define CustomerMAX 200				/*Max possible number of Customers in the System*/
#define OrderTakerMAX 200			/*Max possible number of OrderTakers in the System*/
#define WaiterMAX 200
#define CookMAX 200				
#define TableMAX 200
#define OrderQuantityMAX 6
#define OrdertypeMAX 2

#define NoCooks 200
#define foodthreshold 100

#define SixBurgerPrice 6			/*Price for Six Dollar Burger*/
#define ThreeBurgerPrice 3			/*Price for Three Dollar Burger*/
#define VeggieBurgerPrice 5			/*Price for Veggie Dollar Burger*/
#define FrenchFriesPrice 2			/*Price for French Fries*/
#define SodaPrice 3				/*Price for Soda*/

#define SixBurgerStartingAmount 100		/*The number of Six Dollar Burger initially on the Six Dollar Burger shelf*/
#define ThreeBurgerStartingAmount 100		/*The number of Three Dollar Burger initially on the Three Dollar Burger shelf*/
#define VeggieBurgerStartingAmount 100		/*The number of Veggie Burger initially on the Veggie Burger shelf*/
#define FrenchFriesStartingAmount 100		/*The number of French Fries initially on the French Fries shelf*/

#define ShelfMAX 100				/*Max possible number of items, which the shelf for each Food item type can store
						Example, there can be a max of these many numbers of 6 $ Burger on the              							6 $ Burger Shelf. Each food type has its own shelf*/

#define WaitingOrderThreshold 1	

	

#define UncookedThreshold 30			/*Min no. of uncooked items that must exist in the system before manager orders*/

#define CookedThreshold	30			/*Min no. of cooked items that must exist in the system before manager orders*/

#define CashWithdrawn 2000

#define AddedQuantity	100			/*Quantity added for each order, CONSTANT across the system*/

#define UncookedPrice	20			/*Price for 1 unit of uncooked price*/



int  UncookedStartingAmount=100;		/*The number of Uncooked Stock  initially in the system*/

int  InitialManagerCash=0;	

int baggerQueue;				/*The baggerQueue stores the Token Numbers of orders that need to 
						be bagged. The OrderTaker/Manager appends the Token No. to this queue 
						at the end of his order taking process. The bagger(OrderTaker/Manager)
						removes the Token No. from this queue and uses it to bag the orders
						So, the queue essentially lies between the Entity(who takes the Order and 
						moves on to the next Customer) and the Entity(who bags the orders and at 
						the end, hands it out to Customer himself(if to go) else puts it in the
						waiterQueue for the waiter*/

int waiterQueue;				/*The waiterQueue stores the Token Numbers of orders that need to be served.
						This is the location from where the Waiter can pick up a Token No that needs
						to be attended to. The waiter then finds the Customer and validates the token no*/



int noCustomers, noOrderTakers, noTables, noWaiters,noCooks;
				   		/*Number of Customers & OrderTakers are taken from the user*/
int tokenNo=0;					/*Global Variable for the Token No. Incremented by one each time*/
						/*and then given to both the Customer and the OrderTaker*/

int tokenToId[CustomerMAX];			/*Array stores the Customer ID for each Token No.. Hence, it can be */
						/*used anytime to findout the Customer's ID for a given Token No.*/


						/*The amount of food items currently available on each shelf is initialized*/
						/*to the 'StartingAmount' macro*/
int sixBurgerAvailable;
int threeBurgerAvailable;
int veggieBurgerAvailable;
int frenchFriesAvailable;

int ITEMSINQ=0;
int itemsInWaiterQueue=0;

int noUncooked;					/*The amount of uncooked items currently available is initialized*/
						/*to the 'StartingAmount' macro*/


int customerIDGeneratorLock;
int customerIDGenerator=0;

int orderTakerIDGeneratorLock;
int orderTakerIDGenerator=0;

int waiterIDGeneratorLock;
int waiterIDGenerator=0;

int cookIDGeneratorLock;
int cookIDGenerator=0;

int customerCounterLock;
int customerCounter=0;
	
int randomgenerator;
/*Condition Variables and Lock Variables required for Synchronization*/


/*Monitor for Customer - OrderTaker interaction, the queue part where the customer waits in the Queue for an OrderTaker to be*/
/*assigned to him*/

int custLineLock;          		  	/*Lock for the Customers to avoid Race condition while*/
                    				/*waiting in the Queue for the OrderTakers*/

int custWaitingCV; 		      		/*OrderTaker checks this CV to see if he has to signal */
                    				/*the next Customer*/

int custLineLengthMV=0;            		/*Number of Customers waiting in the Queue. Initialized to 0*/


/*Monitor for the Customer - OrderTaker Interaction, i.e. the Customer is done waiting in the queue and has reached*/
/*his assigned OrderTaker*/

int orderTakerLock[250];		      	/*Each OrderTaker has his own Lock. Array of Locks for */
                    				/*the Customer & OrderTaker interaction  */            

int orderTakerCV[250];			     	/*Each OrderTaker has his own CV. OrderTaker checks this*/
                    				/*CV to see if he has to signal the next Customer*/


/*Monitor for the Bagger - Customer (TO GO TYPE ONLY !) Interaction, i.e. when a Bagger is bagging food for the customer. This
Critical Region includes the time involved in waiting for food, if the required quantity was not available on the shelf.

For To Go Only !!!!


Steps for this Monitor:

1.	The Customer goes to wait after he has placed his order with the OrderTaker 
2.	When the Bagger(OrderTaker in bagging role) has bagged	the Customer's order he Broadcasts to all the To Go 
	customers(from the baggerQueue)
3.	Every Customer who is waiting in this Monitor wakes up one by one and then tries to acquire the lock 
4.	After acquiring lock, the Customer checks each bagger for the token no they have bagged
5.	If the Customer's order has been bagged
		he signals the respective bagger 
mana	else
		the Customer goes to sleep again and waits for the next broadcast
6.	Eventually the bagger gets signaled. His otBagTokenNo gets reset to -1 and he knows that the Customer has
	picked up his order*/


int waitToGoLock[CustomerMAX];		/*Lock for the Customer - Bagger Interaction. Each Customer has his own Lock*/ 
	
int waitToGoCV[CustomerMAX];				

int tokenlock;
int baggerQueueLock;
int waitQueueLock;

/*WAITER Manager monitor*/

int waiterLock;

int waiterCV;

int waitEatInLock;

int waitEatInCV;

int cookManagerLock;
int cookMCV;


/*NEW Locks*/

int uncookedLock;				/*for accessing uncooked items*/
int cookedLock;				/*for accessing either of the cooked items like 6$burger, etc..*/
int otCashLock;				/*for accessing each ordertaker's cash*/


/*Structure Objects Created*/

Cust C[CustomerMAX]; 	      			/*C is an Array of Structures of Cust type*/
OrderTaker OT[OrderTakerMAX];          		/*OT is an Array of Structures of OrderTaker type*/
Waiter W[WaiterMAX];
T1 T[TableMAX];
Manager M;
Cook Coo[CookMAX];






void orderProcessingFn(int i)			/*i is the index of the Customer, i.e. C[i]*/
{
	int x;

	int j;					/*Index Variable for the OrderTaker*/
	j=C[i].myOrderTaker;			/*The OrderTaker used in the function is the one assigned to */
						/*Customer C[i]*/


	/* *****		Order Part Begins	***** */

	/*Order of the Customer C[i] is stored is his array. So the Order details can be obtained just
	by the Customer's ID, i.e. 'i'*/
			
	/*Calculating the bill for Customer C[i]*/
		
	OT[j].otCash+=(C[i].sixBurger * SixBurgerPrice) + (C[i].threeBurger * ThreeBurgerPrice) + (C[i].veggieBurger * VeggieBurgerPrice) + (C[i].frenchFries * FrenchFriesPrice) + (C[i].soda * SodaPrice);
    		

	/* *****		Order Part Ends		***** */	

    	
	/* *****		Token Part Begins	***** */

	/*tokenNo(which's a Global Variable) is given to the Customer & OrderTaker keeps a copy for himself too*/
	Acquire(tokenlock);
	

	C[i].custTokenNo = tokenNo;		/*Token No assigned to the Customer */
	OT[j].otTokenNo = tokenNo;		/*Token No assigned to the OrderTaker*/
	tokenToId[tokenNo++]=i;					
	
	Release(tokenlock);
	

	if (M.mID == j)
	{
        Print("\nCustomer %d is giving order to Manager\n", i, 0,0);
	Print("\nManager is taking order of Customer %d\n", i,0,0);
	

	if (C[i].orderType == 1)	/*To Go*/
		Print("\nCustomer %d receives token number %d from the Manager\n", i, C[i].custTokenNo, 0);
	else				/*eat in*/
		Print("\nCustomer %d is given token number %d by the Manager\n", i, C[i].custTokenNo, 0);
	}

	else
	{
	Print("\nCustomer %d is giving order to OrderTaker %d\n", i, j,0);
	Print("\nOrderTaker %d is taking order of Customer %d\n", j, i,0);
	

	if (C[i].orderType == 1)	/*To Go*/
		Print("\nCustomer %d receives token number %d from the OrderTaker %d\n", i, C[i].custTokenNo, j);
	else				/*eat in*/
		Print("\nCustomer %d is given token number %d by the OrderTaker %d\n", i, C[i].custTokenNo, j);
	}
	
	/*Token No assigned and incremented*/
    	

	/* *****		Token Part Ends		***** */



	/*TO GO PART BEGINS ************** */
	
	if (C[i].orderType == 1)
	{
/*
		Acquire(waitToGoLock[i]);			/*Customer C[i] acquires its lock for the Bagger - To Go Customer */
/*	        Acquire(baggerQueueLock); 
 /*
		AppendList(baggerQueue, OT[j].otTokenNo);		/*The Order which needs to be bagged, */


/*		ITEMSINQ++;			
                Release(baggerQueueLock);     				/*its Token No. is appended to the bagger queue */



	
		
    		/* *****		Monitor for TO GO Customers Begins	***** */




	if (C[i].toGoWaiting==0)
		C[i].toGoWaiting=1;				/*Customer C[i]'s toGoWaiting flag is set now, */
		
	/*Release(orderTakerLock[j]);		

	Wait(waitToGoCV[i], waitToGoLock[i]);		/*Customer  C[i] waits for the order to be bagged and */
			


	while(C[i].toGoWaiting == 1 && C[i].custDoA == 1) 

	{		
		for (x=0; x < noOrderTakers; x++)			/*Checks each OrderTaker */
		{
			if (OT[x].otBagTokenNo == C[i].custTokenNo && C[i].custDoA == 1 )	/*Sees if the bagged order's token */ 													/*no matches his own token */
												/*no */
			{
				 Signal(waitToGoCV[i], waitToGoLock[i]);

				C[i].toGoWaiting=0;
				C[i].custDoA = 0;
	

				OT[x].otBagTokenNo = -1;		/*Token no matches. So OT[x]'s bagging flag is reset to -1 */
		
	
				Release(waitToGoLock[i]);		/*Can release the lock now as the customer has */
 			
				return;					/*Exits the orderProcessingfn()	*/		
				}
	
			
		}
	
		if (C[i].custDoA == 1){

		Wait(waitToGoCV[i], waitToGoLock[i]);		/*Customer  C[i] waits for the order to be bagged and*/
								/*the order to be bagged and */
			}
								/*the signal from the bagger*/	

}

	}								/*if TO GO ends here*/

    		/* *****		Monitor for TO GO Customers Ends	***** */

		/* *****		Monitor for EAT IN Customers Begins	***** */
	
	else if(C[i].orderType==0)
	{       
               /* Acquire(baggerQueueLock);
		
                AppendList(baggerQueue, OT[j].otTokenNo);	/*The Order which needs to be bagged,*/

                ITEMSINQ++;
		Release(baggerQueueLock);
                Release(orderTakerLock[j]);		

		/*Wait for acquiring the table*/
		
		Acquire(waitEatInLock);		/*Acquires lock for waiting on the waiter*/


		Print("\nCustomer %d is waiting for the waiter to serve the food\n", i,0,0);
		
		Print("\nWaiter 0 validates the token number for Customer %d\n",i,0,0);
		Print("\nWaiter 0 serves food to Customer %d\n", i,0 ,0);
                Print("\nCustomer %d is served by waiter 0\n", i, 0,0);				
	        Print("\nCustomer %d is leaving the restaurant after having food\n", i, 0,0);

	
/*
		Wait(waitEatInCV,waitEatInLock);	/*Waiting for waiter's broadcast*/

		
		while (1)
		{
			for (x=0; x < noTables; x++)
				if (T[x].tStatus == TFree)
				{
	
					C[i].custTableID=x;
					T[x].tStatus=TBusy;
					break;
				}
		
			if (C[i].custTableID == -1)
			{
			/*	Print("Waiting for table****************************",0,0,0);
				Yield() */ ;
			}
			else
				break;

		}

	
		/*By the time the loop ends, C[i] has been assigned a table*/
		/*Customer's Table ID has been set and Table's status is changed*/

		Print("\nCustomer %d is seated at table number %d\n", i, C[i].custTableID,0);	
							
	
		/*The time taken by the customer on the table, once he reaches there*/
		
                /*
		for (x=0; x<1; x++)	/*Customers eat quickly*/
	/*		Yield();
*/
		
		/*Customer is done eating*/
		
		T[C[i].custTableID].tStatus=TFree;


		
		/*Customer has received the broadcast and now is awake and has the waitEatInLock*/
				
		while (C[i].waiterAssigned == -1)
		{	
			for (x=0; x < noWaiters; x++)
			{
				if (W[x].wTokenNo == C[i].custTokenNo)		/*Broadcast was for this customer's TokenNo*/
				{
					 Signal(waitEatInCV,waitEatInLock);	/*Signals the waiter that the token no has matched*/
				
					C[i].waiterAssigned=x;
		
					Print("\nWaiter %d validates the token number for Customer %d\n",x,i,0);
					Print("\nWaiter %d serves food to Customer %d\n", x, i,0);
                                	Print("\nCustomer %d is served by waiter %d\n", i, x,0);				
					Print("\nCustomer %d is leaving the restaurant after having food\n", i, 0,0);

					Acquire(customerCounterLock);
		customerCounter++;
		Release(customerCounterLock);
					break;	
				}
			}

			
			/*Token didn't match*/
			
			 Wait(waitEatInCV,waitEatInLock);	/*Waiting again for waiter's broadcast*/
		}
		
		
		/*Waiter Assigned by now*/
	
		Release(waitEatInLock);			/*Acquires lock for waiting on the waiter*/



		C[i].custDoA=0;
				
		
	}
	/* *****		Monitor for EAT IN Customers Ends	***** */




}		




/*Function for the Customer where 'i' is the Customer ID*/




void main()
{
	int i;
	
	int j;

	
	

	Acquire(customerIDGeneratorLock);
	i=customerIDGenerator++;		
	Release(customerIDGeneratorLock);



	/* *****			Initialization Part Begins		***** */
    	/*ith Customer's Structure variables are initialized. For information unavailable right now, -1 value is assigned */
    
    	C[i].custID=i;                			/*Customer's ID - custID goes like 1, 2, 3, ....*/
    	C[i].myOrderTaker=-1;        			/*Initialized to -1 as no OrderTaker assigned right now*/
    	C[i].custTokenNo=-1;           			/*Initialized to -1 as no Token No. given right now*/
 /*   	C[i].orderStatus=0;*/				/*Initially orderStatus is set to 0, i.e. Not Served Yet*/

	C[i].custTableID=-1;
	C[i].waiterAssigned=-1;
	C[i].toGoWaiting=0;
	C[i].custDoA=1;
	
	if (randomgenerator == 1)
		C[i].orderType=1;
	else
		C[i].orderType=0;


/*rand()%OrdertypeMAX*/			/*Order Type is randomly generated as Eat In or To Go using rand()*/
    	C[i].sixBurger=3/*rand() % OrderQuantityMAX*/;      	/*6$ Burger ordered or not?? 0 for No, else quantity is specified*/
 	C[i].threeBurger=3/*rand() % OrderQuantityMAX*/;    	/*3$ Burger ordered or not?? 0 for No, else quantity is specified*/
    	C[i].veggieBurger=3/*rand() % OrderQuantityMAX*/;  	/*Veggie Burger ordered or not?? 0 for No, else quantity is specified*/
    	C[i].frenchFries=3/*rand() % OrderQuantityMAX*/;    	/*French Fries ordered or not?? 0 for No, else quantity is specified*/
    	C[i].soda=0;
	/*rand() % OrderQuantityMAX;   */ 	/*Soda ordered or not?? 0 for No, else quantity is specified */


	waitToGoLock[i]=CreateLock("waitToGo_Lock", 13);		/*waitToGoLock Created*/
	waitToGoCV[i]=CreateCondition("waitToGo_CV", 11);		/*waitToGoCV Created*/




	/* *****			Initialization Part Ends		***** */


    	/*Customer enters the system. Acquires custLineLock and holds it till he reaches the OrderTaker */

	Acquire(custLineLock);	        			/*Customer acquires custLineLock */
	
			/*Customer Order Type displayed */
	
	if (C[i].orderType == 1)
		Print("\nCustomer %d chooses to to-go the food\n",i,0,0); 			
	else
		Print("\nCustomer %d chooses to eat-in the food\n",i,0,0); 
		
	
   
if(C[i].sixBurger >0)
{     	Print("\nCustomer %d is ordering 6-dollar burger\n",i,0,0);
}
if(C[i].sixBurger == 0)
{        Print("\nCustomer %d is not ordering 6-dollar burger\n",i,0,0);          
}
if(C[i].threeBurger >0)
{     	Print("\nCustomer %d is ordering 3-dollar burger\n",i,0,0);
}
if(C[i].threeBurger == 0)
{     	Print("\nCustomer %d is not ordering 3-dollar burger\n",i,0,0);
}
if(C[i].veggieBurger >0)
{     	Print("\nCustomer %d is ordering veggie burger\n",i,0,0);
}
if(C[i].veggieBurger == 0)
{     	Print("\nCustomer %d is not ordering veggie burger\n",i,0,0);
}      
if(C[i].frenchFries >0)
{     Print("\nCustomer %d is ordering french fries\n",i,0,0);
}
if(C[i].frenchFries ==0)
{     Print("\nCustomer %d is not ordering french fries\n",i,0,0);
}

	for(j=0; j < noOrderTakers; j++)    			/*For each OrderTaker */
        	if(OT[j].otStatusMV == OTFree)    		/*Customer checks if OT[j] is Free */
        	{  
			OT[j].otStatusMV=OTBusy;    		/*OT[j] was found Free & is now set to Busy*/
            		C[i].myOrderTaker=j;    		/*OT[j] has now been assigned to the Customer*/
			break;    
        	}
	

     	if(C[i].myOrderTaker == -1)         			/*no one's available, get in line*/    
    	{
		custLineLengthMV++; 				/*Customer is now waiting in line, so custLineLengthMV*/
		Wait(custWaitingCV, custLineLock);		/*wait for OrderTaker to be available*/
		
	}

	
	/*If Customer C[i] has reached here, it means that it was signaled by an OrderTaker.As any Customer can go */
	/*to and OrderTaker in the 'Waiting' State, the Customer goes to the first OrderTaker that it finds 'waiting'*/

    	for(j=0; j < noOrderTakers; j++)    			/*For each OrderTaker*/
    	{
        	if(OT[j].otStatusMV == OTWaiting)    		/*note: if this condition fails, we know it must be a */
   		{						/*Manager doing the order taking*/
	     		C[i].myOrderTaker = j;         		/*jth OrderTaker is assigned to C[i] now*/
			OT[j].otStatusMV = OTBusy;		/*Now the OrderTaker is Busy*/
			break;


/*TEMP:No order bagging yet!! So, once OrderTaker has been assigned to the Customer, assumed that he has been served */ 		       	
	/*		OT[j].otStatusMV=OTFree;*/  		/*REMOVE LATER*/


   		}     	
    	}


 	/*By now the Customer is done waiting in the queue and ready to place the order*/
	
       	Release(custLineLock);         				/*As the Customer is no longer in the queue*/
		        				
	Acquire(orderTakerLock[C[i].myOrderTaker]);   	 	/*Customer is now ready to give his order*/
	Signal(orderTakerCV[C[i].myOrderTaker], orderTakerLock[C[i].myOrderTaker]);	/*Customer C[i] signals its OrderTaker */
	
	orderProcessingFn(i);					/*The order function for Customer C[i] is called*/	
								/*Function takes care of order, payment, assignment of token*/
								/*and appending the token no in the bagger queue part*/
	Exit(0);	

}


