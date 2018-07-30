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


/*Manager goes to ATM Fn*/
void goToBankFn()
{
	int x;

/*	for (x=0; x < (rand() % 5) ; x++)
*/		Yield();
	
	Print("\nManager goes to bank to withdraw the cash\n",0,0,0);

	M.mCash+=CashWithdrawn;					/*Manager gets richer by the CashWithdrawn Amount*/
}



/*Function for Bagging Food Process*/
								
void baggingFn(int j, int bagTokenNo)						/*j is the index of the Bagger, i.e. OT[j]*/
{
	int i;							/*For the index of the Customer, i.e. C[i]*/
	int x;

	for ( x=0; x<noCustomers; x++)
	
	/*TO GO part*/
	if (C[x].toGoWaiting == 1 && C[x].custDoA == 1 && C[x].orderType==1)
	{	
		
		 Acquire(waitToGoLock[x]);			/*Bagger acquires the lock of Customer C[i] for the Bagger -*/
			 
	}



	i=tokenToId[bagTokenNo];				/*Obtained Customer ID from the Bag Token No*/
	

	OT[j].otBagTokenNo=bagTokenNo;
	

/*	while (sixBurgerAvailable < C[i].sixBurger || threeBurgerAvailable < C[i].threeBurger || veggieBurgerAvailable < C[i].veggieBurger || frenchFriesAvailable < C[i].frenchFries) currentThread -> Yield();*/

	Acquire(cookedLock);
	sixBurgerAvailable-=C[i].sixBurger;			/*The amount of food item of each type available*/
	threeBurgerAvailable-=C[i].threeBurger;			/*on its respective shelf is decremented by the number*/
	veggieBurgerAvailable-=C[i].veggieBurger;		/*of items of that type ordered by the Customer C[i]*/
	frenchFriesAvailable-=C[i].frenchFries;
	Release(cookedLock);	
	
		

	if (C[i].orderType == 1)
	{
	
    		/* *****		Monitor for TO GO Customers Begins	***** */

		if (M.mID==j){
		Print("\nManager packed the food for Customer %d", i,0, 0);
		Print("\nManager gives food to Customer %d", i,0, 0);
		Print("\nCustomer %d receives food from the Manager\n", i, 0,0);
		Print("\nCustomer %d is leaving the restaurant after Manager packed the food\n", i, 0,0);
		Acquire(customerCounterLock);
		customerCounter++;
		Release(customerCounterLock);

		}
		else
		{
		Print("\nOrderTaker %d packed the food for Customer %d", j, i,0);
		Print("\nOrderTaker %d gives food to Customer %d", j, i,0);
		Print("\nCustomer %d receives food from the OrderTaker %d\n", i, j,0);
		Print("\nCustomer %d is leaving the restaurant after OrderTaker %d packed the food\n", i, j,0);

		Acquire(customerCounterLock);
		customerCounter++;
		Release(customerCounterLock);


		}


		if (C[i].toGoWaiting==1);
		{	
			Signal(waitToGoCV[i], waitToGoLock[i]);		
	
		}
	
	for (x=0; x < noCustomers; x++)			/*For each Customer*/
		{
		/*	if(waitToGoLock[x]->isHeldByCurrentThread() && x!=i)*/
			if (x !=i)		
		{
			Release(waitToGoLock[x]);
			}
		}
	
	
	 Wait(waitToGoCV[i], waitToGoLock[i]);
	Release(waitToGoLock[i]);
		
			
		return;							/*Exits fn*/
	}


	

    		/* *****		Monitor for TO GO Customers Ends	***** */
	
	
	else
	{
		/*If the control reaches here, the order is Eat in type. Therefore, the Token No is appended to the waiterQueue
    		For Eat In Customers
		*/
		
		Print("\nOrderTaker %d gives token number %d to Customer %d", C[i].myOrderTaker, C[i].custTokenNo, i);
		
               	Acquire(waitQueueLock);				/*Ensures that only Bagger or Waiter use the queue at a time*/

		AppendList(waiterQueue, bagTokenNo);		/*The Order which needs to be bagged,
									its Token No. is appended to the waiter queue
								*/
		itemsInWaiterQueue++;
                
		Release(waitQueueLock);
                

	}

	
	/*return back to orderTakerFn*/	
						

						
}







void main()
{




	int i;
	int x;		/*for loop index*/
	int seed;	/*for generating random amount of wait time*/
	int bagTokenNo;
	int noCustomersServed=0;


	M.mCash=InitialManagerCash;
	M.mID=noOrderTakers ;

	/*Manager as an OrderTaker Initialization*/

	/*OrderTaker OT[i] becomes active only when manager works as an ordertaker*/	


	/*MANAGER ENTERS SYSTEM*/

	while (1)
	{
		Acquire(customerCounterLock);

		if (customerCounter == noCustomers)
		{
			for (x=0; x<1000; x++)
				Yield();

		Halt();
		}
		Release(customerCounterLock);

			/* ************MANAGER AS AN ORDERTAKER STARTS HERE*************** */
			
		
		if (custLineLengthMV > 3)		/*More than 3 customers waiting*/
		{

			noOrderTakers++;	
		
					orderTakerLock[M.mID]=CreateLock("OrderTaker_Lock",15);		/*orderTakerLock Created*/
					orderTakerCV[M.mID]=CreateCondition("Order_Taker_CV",14);	/*orderTakerCV Created*/

		
			OT[M.mID].otStatusMV=OTBusy;		
			OT[M.mID].otID=M.mID;                			/*OrderTaker's ID - otID goes like 1, 2, 3, ....*/
   			OT[M.mID].otTokenNo=-1;				/*Initialized to -1 as no Token No. given right now*/
			OT[M.mID].otCash=0;
		
			/* **************START OF ORDERTAKER CODE************* */
			
			

			while(noCustomersServed <= 2)           	
     			{							/*OT works continuously, checking to see if he has*/  											/*served 2 customers*/
    				 Acquire(custLineLock); 			/*OT acquires custLineLock so that he can */
										/*later signal the waiting Customer*/

			     				
        			if (custLineLengthMV > 0)        		/*i.e. someone's waiting in the line*/
        			{      
					custLineLengthMV--; 			/*CustLineLength decremented as a */
										/*customer has ended witing in line*/
					OT[M.mID].otStatusMV=OTWaiting;		/*OT[i] is now in 'Waiting' state as it */
										/*has already signaled */
					
                       			 Signal(custWaitingCV, custLineLock); 	/*OT Signals Customer*/
	

        			}

       				else if (ITEMSINQ > 0)
        			{
					
					noCustomersServed++;

                    		    	Acquire(baggerQueueLock);			
            				bagTokenNo=RemoveList(baggerQueue);
					ITEMSINQ--;
                        		Release(baggerQueueLock); 

			
	     				Release(custLineLock);        /*Food is now bagged so the custLineLock is released*/
					
                       
					baggingFn(M.mID, bagTokenNo);		/*The bagging function takes care of */		

		
            				continue;                		/*OrderTaker will continously bag food if */
										/*there are any food	*/
               	                 						/*items that need to be bagged*/
        			}

     	  		
				
				 Acquire(orderTakerLock[M.mID]);    		/*OrderTaker i should enter the Critical */
										/*Region of Customer-Order*/
                     								/*taker interaction before the Customer does*/
			/*	if (custLineLock -> isHeldByCurrentThread())*/
    				Release(custLineLock);    			/*OrderTaker is now in the Customer-OrderTaker*/
										/* Critical Region	*/
					    				       	/*hence, the custLineLock is no longer required*/
			
				 Wait(orderTakerCV[M.mID],orderTakerLock[M.mID]);	/*OrderTaker i is waiting for Customer's */
 			
				noCustomersServed++;

				Release(orderTakerLock[M.mID]);    		/*OrderTaker i releases the OrderTakerLock*/
	
			}                  					/*while(true) loop ends here */               
			

			noOrderTakers--;					/*OT[i] is inactive now*/
			OT[M.mID].otStatusMV=OTFree;		

		}								/*if condition ends here*/		
	

			/* *Manager as OrderTaker ends here....... */
		 if(sixBurgerAvailable<=foodthreshold)
		{
			
			for(i=0;i<noCooks;i++)
			{
				if(Coo[i].Status==0)
					{
						Coo[i].Status=1;
						Print("\nManager informs Cook %d to cook 6-dollar burger\n", Coo[i].cookid,0,0);
						Print("\nCook %d is going to cook 6-dollar burger\n", Coo[i].cookid,0,0);
						Acquire(cookManagerLock);
						Signal(cookMCV,cookManagerLock);
						Release(cookManagerLock);

						break;
					}
				else if(Coo[i].Status==2)
					{	
						Coo[i].Status=1;
						Print("Cook %d returned from break", Coo[i].cookid,0,0);
						Print("\nManager informs Cook %d to cook 6-dollar burger\n", Coo[i].cookid,0,0);
						Print("\nCook %d is going to cook 6-dollar burger\n", Coo[i].cookid,0,0);
						Acquire(cookManagerLock);
						Signal(cookMCV,cookManagerLock);
						Release(cookManagerLock);
						break;
									
					}
				else
					{	
						Yield();
						break;
					}
			}      	
		}
	else 
		/* compare the value with the specific amount in the inventory. make cook Sleep*/
		{
			for(i=0;i<noCooks;i++)
			{
				if(Coo[i].Status==0)
				{
					Coo[i].Status==2;
					Print("Cook %d is going on break", Coo[i].cookid,0,0);

					break;
				}
			}
		}
		
			
		if(threeBurgerAvailable<=foodthreshold)
		{
		for(i=0;i<noCooks;i++)
			{
					if(Coo[i].Status==0)
					{
						Coo[i].Status=1;
						Print("\nManager informs Cook %d to cook 3-dollar burger\n", Coo[i].cookid,0,0);
						Print("\nCook %d is going to cook 3-dollar burger\n", Coo[i].cookid,0,0);
						Acquire(cookManagerLock);
						Signal(cookMCV,cookManagerLock);
						Release(cookManagerLock);
					
						break;
					}
					else if(Coo[i].Status==2)
					{
						Coo[i].Status=1;
						Print("Cook %d returned from break", Coo[i].cookid,0,0);
						Print("\nManager informs Cook %d to cook 3-dollar burger\n", Coo[i].cookid,0,0);
						Print("\nCook %d is going to cook 3-dollar burger\n", Coo[i].cookid,0,0);
						Acquire(cookManagerLock);
						Signal(cookMCV,cookManagerLock);
						Release(cookManagerLock);
					
						break;				
					}
				     else
					{
						Yield();
						break;
					}
			}      	
		}
	
		else
		{
			for(i=0;i<noCooks;i++)
			{
				if(Coo[i].Status==0)
				{
					Coo[i].Status==2;
					Print("Cook %d is going on break", Coo[i].cookid,0,0);
					
					break;	
				}
			}
		}
			


		if(veggieBurgerAvailable<=foodthreshold)
		{
		for(i=0;i<noCooks;i++)
			{
				if(Coo[i].Status==0)
					{
						Coo[i].Status=1;
						Print("\nManager informs Cook %d to cook veggie burger\n", Coo[i].cookid,0,0);
						Print("\nCook %d is going to cook veggie burger\n", Coo[i].cookid,0,0);
						Acquire(cookManagerLock);
						Signal(cookMCV,cookManagerLock);
						Release(cookManagerLock);
				
						break;
					}
				else if(Coo[i].Status==2)
					{
						Coo[i].Status=1;
						Print("Cook %d returned from break", Coo[i].cookid,0,0);
						Print("\nManager informs Cook %d to cook veggie burger\n", Coo[i].cookid,0,0);
						Print("\nCook %d is going to cook veggie burger\n", Coo[i].cookid,0,0);
						Acquire(cookManagerLock);
						Signal(cookMCV,cookManagerLock);
						Release(cookManagerLock);
				
						break;					
					}
				else
					{
						Yield();
						break;
					}
			}      	
		}
	
	else 
		{
			for(i=0;i<noCooks;i++)
			{
				if(Coo[i].Status==0)
				{
					Coo[i].Status==2;
					Print("Cook %d is going on break", Coo[i].cookid,0,0);
				
					break;
				}
			}
		}
		
		if(frenchFriesAvailable<=foodthreshold)
		{
		for(i=0;i<noCooks;i++)
			{
				if(Coo[i].Status==0)
					{
						Coo[i].Status=1;
						Print("\nManager informs Cook %d to cook french fries\n", Coo[i].cookid,0,0);
						Print("\nCook %d is going to cook french fries\n", Coo[i].cookid,0,0);				Acquire(cookManagerLock);
						Signal(cookMCV,cookManagerLock);
						Release(cookManagerLock);
				
						break;
					}
				else if(Coo[i].Status==2)
					{
						Coo[i].Status=1;
						Print("Cook %d returned from break", Coo[i].cookid,0,0);
						Print("\nManager informs Cook %d to cook french fries\n", Coo[i].cookid,0,0);
						Print("\nCook %d is going to cook french fries\n", Coo[i].cookid,0,0);					Acquire(cookManagerLock);
						Signal(cookMCV,cookManagerLock);
						Release(cookManagerLock);
				
						break;					
					}
				else
					{
						Yield();
						break;
					}
			}      	
		}
	else 
		/* compare the value with the specific amount in the inventory. make cook Sleep*/
		{
			for(i=0;i<noCooks;i++)
			{
				if(Coo[i].Status==0)
				{
					Coo[i].Status==2;
					Print("Cook %d is going on break", Coo[i].cookid,0,0);
				
					break;
				}
			}
		}


			
		
		/* ************MANAGER AS AN ORDERTAKER ENDS HERE***************  */



		
		/* ***************MAINTAINING UNCOOKED STARTS******************* */		
	 
		 Acquire(uncookedLock);		/*accessing noUncooked variable */
	
		if (noUncooked < UncookedThreshold)	/*if current uncooked < threshold */
		{
						
			/*check if cash available after collecting each OT's cash, i.e. removes each of the OT's cash & */
			/*adds it to his own cash */

			 Acquire(otCashLock);
			
			for (x=0; x < noOrderTakers; x++)
			{
				M.mCash+=OT[x].otCash;
				OT[x].otCash=0;
			}
	
			Release(otCashLock);			
						
			/* by this time, mCash has total cash in the system */

			/*cash reqd for this order=AddedQuantity * UncookedPrice*/

			while (noUncooked < UncookedThreshold)
			{
				
				if (M.mCash >  AddedQuantity * UncookedPrice )
				{
					 Release(uncookedLock);
					/*enough cash to place an order */
				
					/*order placed now, so cash decremented */
					M.mCash -=AddedQuantity * UncookedPrice;
			
					Print("\nManager refills the inventory\n",0,0,0);

					/*Order placed !!*/
				
					/*Time waiting for order to come*/
				
				/*	while (1)		*/		/*seed is 1 or 2*/
					/*	if (!(seed=rand() % 2))	*/	/*CAN TAKE MACRO IF TIME LATER !!!!*/
				/*			break;
*/

					for (x=1; x <= seed * 5 * 2 ; x++)
						Yield();
						
					/*Wait is now over!!*/

					Acquire(uncookedLock);
					noUncooked+=AddedQuantity;

					Print("\nInventory is loaded in the restaurant\n",0,0,0);
				
				/*	if (uncookedLock -> isHeldByCurrentThread())	*/	/*NOT REQD but for backup*/
			 Release(uncookedLock);
				}

				else 
				{
				/*	if (uncookedLock -> isHeldByCurrentThread())	*/	/*NOT REQD but for backup*/
			Release(uncookedLock);
					/*NO Cash*/
					/*Needs to go to the ATM*/
					goToBankFn();
					Acquire(uncookedLock);
					/*Manager is now richer by CashWithdrawn*/
				}
			}						/*while loop ends here*/
		}							/*uncooked items ordered*/

	/*	if (uncookedLock -> isHeldByCurrentThread())	*/	/*NOT REQD but for backup*/
			Release(uncookedLock);
		

		/* ***************MAINTAINING UNCOOKED ENDS******************* */


		
		/* ******************************Waiter Manager Interaction */

		
		if(itemsInWaiterQueue > WaitingOrderThreshold)		
		{
			
			 Acquire(waiterLock);		/*Manager Waiter interaction Lock */	
	
			

			Print("\nManager calls back all Waiters from break\n",0,0,0);			

			 Signal(waiterCV,waiterLock);	
			
			Release(waiterLock);
		}
		Yield();
	}







}
