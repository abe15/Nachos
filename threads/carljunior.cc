                                             

//Simulation of Carl's Junior Restaurant

//CV denotes the Condition Variables & MV denotes the Monitor Variables

#include "copyright.h"
#include "system.h"
#ifdef CHANGED
#include "synch.h"
#include "carljunior.h"


//Global Variables

#define CustomerMAX 200				//Max possible number of Customers in the System
#define OrderTakerMAX 200			//Max possible number of OrderTakers in the System
#define WaiterMAX 200
#define CookMAX 200				
#define TableMAX 200
#define OrderQuantityMAX 6
#define OrdertypeMAX 2

#define NoCooks 200
#define foodthreshold 100

#define SixBurgerPrice 6.00			//Price for Six Dollar Burger
#define ThreeBurgerPrice 3.00			//Price for Three Dollar Burger
#define VeggieBurgerPrice 4.75			//Price for Veggie Dollar Burger
#define FrenchFriesPrice 1.50			//Price for French Fries
#define SodaPrice 2.50				//Price for Soda

#define SixBurgerStartingAmount 100		//The number of Six Dollar Burger initially on the Six Dollar Burger shelf
#define ThreeBurgerStartingAmount 100		//The number of Three Dollar Burger initially on the Three Dollar Burger shelf
#define VeggieBurgerStartingAmount 100		//The number of Veggie Burger initially on the Veggie Burger shelf
#define FrenchFriesStartingAmount 100		//The number of French Fries initially on the French Fries shelf

#define ShelfMAX 100				/*Max possible number of items, which the shelf for each Food item type can store
						Example, there can be a max of these many numbers of 6 $ Burger on the              							6 $ Burger Shelf. Each food type has its own shelf*/

#define WaitingOrderThreshold 1	

	

#define UncookedThreshold 30			//Min no. of uncooked items that must exist in the system before manager orders

#define CookedThreshold	30			//Min no. of cooked items that must exist in the system before manager orders

#define CashWithdrawn 2000.00

#define AddedQuantity	100			//Quantity added for each order, CONSTANT across the system

#define UncookedPrice	20.00			//Price for 1 unit of uncooked price


int  UncookedStartingAmount=100;		//The number of Uncooked Stock  initially in the system

float  InitialManagerCash=0.00;	

List *baggerQueue;				/*The baggerQueue stores the Token Numbers of orders that need to 
						be bagged. The OrderTaker/Manager appends the Token No. to this queue 
						at the end of his order taking process. The bagger(OrderTaker/Manager)
						removes the Token No. from this queue and uses it to bag the orders
						So, the queue essentially lies between the Entity(who takes the Order and 
						moves on to the next Customer) and the Entity(who bags the orders and at 
						the end, hands it out to Customer himself(if to go) else puts it in the
						waiterQueue for the waiter*/

List *waiterQueue;				/*The waiterQueue stores the Token Numbers of orders that need to be served.
						This is the location from where the Waiter can pick up a Token No that needs
						to be attended to. The waiter then finds the Customer and validates the token no*/

int choice;

int noCustomers, noOrderTakers, noTables, noWaiters,noCooks;
				   		//Number of Customers & OrderTakers are taken from the user
int tokenNo=0;					//Global Variable for the Token No. Incremented by one each time
						//and then given to both the Customer and the OrderTaker

int tokenToId[CustomerMAX];			//Array stores the Customer ID for each Token No.. Hence, it can be 
						//used anytime to findout the Customer's ID for a given Token No.


						//The amount of food items currently available on each shelf is initialized
						//to the 'StartingAmount' macro
int sixBurgerAvailable=SixBurgerStartingAmount;
int threeBurgerAvailable=ThreeBurgerStartingAmount;
int veggieBurgerAvailable=VeggieBurgerStartingAmount;
int frenchFriesAvailable=FrenchFriesStartingAmount;

int ITEMSINQ=0;
int itemsInWaiterQueue=0;

int noUncooked=UncookedStartingAmount;		//The amount of uncooked items currently available is initialized
						//to the 'StartingAmount' macro

int customerCounter=0;
Lock *customerCounterLock;

//Structure Objects Created

Cust C[CustomerMAX]; 	      			//C is an Array of Structures of Cust type
OrderTaker OT[OrderTakerMAX];          		//OT is an Array of Structures of OrderTaker type
Waiter W[WaiterMAX];
T1 T[TableMAX];
Manager M;
Cook Coo[CookMAX];



//Condition Variables and Lock Variables required for Synchronization


//Monitor for Customer - OrderTaker interaction, the queue part where the customer waits in the Queue for an OrderTaker to be
//assigned to him

Lock *custLineLock;          		  	//Lock for the Customers to avoid Race condition while
                    				//waiting in the Queue for the OrderTakers

Condition *custWaitingCV;       		//OrderTaker checks this CV to see if he has to signal 
                    				//the next Customer

int custLineLengthMV=0;            		//Number of Customers waiting in the Queue. Initialized to 0


//Monitor for the Customer - OrderTaker Interaction, i.e. the Customer is done waiting in the queue and has reached
//his assigned OrderTaker

Lock *orderTakerLock[250];      	//Each OrderTaker has his own Lock. Array of Locks for 
                    				//the Customer & OrderTaker interaction              

Condition *orderTakerCV[250];     	//Each OrderTaker has his own CV. OrderTaker checks this
                    				//CV to see if he has to signal the next Customer


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
	else
		the Customer goes to sleep again and waits for the next broadcast
6.	Eventually the bagger gets signaled. His otBagTokenNo gets reset to -1 and he knows that the Customer has
	picked up his order*/


Lock *waitToGoLock[CustomerMAX];		//Lock for the Customer - Bagger Interaction. Each Customer has his own Lock 
	
Condition *waitToGoCV[CustomerMAX];				


//WAITER Manager monitor

Lock *waiterLock;

Condition *waiterCV;

Lock *waitEatInLock;

Condition *waitEatInCV;

Lock *baggerQueueLock;
Lock *waitQueueLock;

Lock *cookManagerLock;
Condition *cookMCV;


//NEW Locks

Lock *uncookedLock;				//for accessing uncooked items
Lock *cookedLock;				//for accessing either of the cooked items like 6$burger, etc..
Lock *otCashLock;				//for accessing each ordertaker's cash



/*Function for Order Process of Customer C[i]

Function performs the following tasks:
1.	Gets the Customer's order from the variables used in the Customer's Structure
2.	Calculates the billing amount for the Customer 
3.	OrderTaker increments the cash it holds by the billing amount
4.	OrderTaker give the Customer its Token No and keeps a copy for himself as well
5. 	OrderTaker appends the Token No to the baggerQueue, which is used by the bagger to get the token 
	numbers of orders that need to be bagged*/

void cookFn(int c)
{
	
	

        Coo[c].cookid=c;			//assigning an id to the cook.
	

	
		
	cookManagerLock->Acquire();		//acquiring the cook and manager lock.
	
	cookMCV->Wait(cookManagerLock);	//Initiating the condition variable and waiting ofr the signal from manager.
	

	
	for(int i=0;i<noCooks;i++)
	{
		if(Coo[i].Status==0)
		{
	         cookManagerLock->Acquire();		//acquiring the cook and manager lock.
	
		cookMCV->Wait(cookManagerLock);	//Initiating the condition variable and waiting ofr the signal from manager.
		}
	}

	
	
	for(int i=0;i<noCooks;i++)
	{
		if(Coo[i].Status==2)
		{	cookManagerLock->Acquire();		//acquiring the cook and manager lock.
	
			cookMCV->Wait(cookManagerLock);	//Initiating the condition variable and waiting ofr the signal from manager.
		}

	}
	
	if(sixBurgerAvailable<=foodthreshold)
		{
			sixBurgerAvailable+=100;
			otCashLock -> Acquire();
			M.mCash= M.mCash - 100;
			otCashLock -> Release();
			uncookedLock -> Acquire();
			noUncooked-=20;
			uncookedLock->Release();
			
		}

	if(threeBurgerAvailable<=foodthreshold)
		{
			threeBurgerAvailable+=100;
			otCashLock -> Acquire();
			M.mCash= M.mCash - 70;
			otCashLock -> Release();
			
			uncookedLock -> Acquire();
			noUncooked-=20;
			uncookedLock->Release();
			
		}
	
	if(veggieBurgerAvailable<=foodthreshold)
		{
			veggieBurgerAvailable+=100;
			otCashLock -> Acquire();
			M.mCash= M.mCash - 50;
			otCashLock -> Release();
			uncookedLock -> Acquire();
			noUncooked-=20;
			uncookedLock->Release();
		}
	if(frenchFriesAvailable<=foodthreshold)
		{
			frenchFriesAvailable+=100;
			otCashLock -> Acquire();
			M.mCash= M.mCash - 30;
			otCashLock -> Release();
			uncookedLock -> Acquire();
			noUncooked-=20;
			uncookedLock->Release();
		}
	cookManagerLock->Release();
}

void orderProcessingFn(int i)			//i is the index of the Customer, i.e. C[i]
{
	int x;

	int j;					//Index Variable for the OrderTaker
	j=C[i].myOrderTaker;			//The OrderTaker used in the function is the one assigned to 
						//Customer C[i]


	// *****		Order Part Begins	***** //

	/*Order of the Customer C[i] is stored is his array. So the Order details can be obtained just
	by the Customer's ID, i.e. 'i'
			
	Calculating the bill for Customer C[i]*/

	OT[j].otCash+=(C[i].sixBurger * SixBurgerPrice) + (C[i].threeBurger * ThreeBurgerPrice) + (C[i].veggieBurger * VeggieBurgerPrice) + (C[i].frenchFries * FrenchFriesPrice) + (C[i].soda * SodaPrice);
    	
	

	// *****		Order Part Ends		***** //	

    	
	// *****		Token Part Begins	***** //

	//tokenNo(which's a Global Variable) is given to the Customer & OrderTaker keeps a copy for himself too
	C[i].custTokenNo = tokenNo;		//Token No assigned to the Customer 
	OT[j].otTokenNo = tokenNo;		//Token No assigned to the OrderTaker
	tokenToId[tokenNo]=i;					
	
	if (M.mID == j)
	{
        printf("\nCustomer %d is giving order to Manager %d\n", i, j);
	printf("\nManager %d is taking order of Customer %d\n", j, i);
	

	if (C[i].orderType == 1)	//To Go
		printf("\nCustomer %d receives token number %d from the Manager %d\n", i, C[i].custTokenNo, j);
	else				//eat in
		printf("\nCustomer %d is given token number %d by the Manager %d\n", i, C[i].custTokenNo, j);
	}

	else
	{
		printf("\nCustomer %d is giving order to OrderTaker %d\n", i, j);
	printf("\nOrderTaker %d is taking order of Customer %d\n", j, i);
	

	if (C[i].orderType == 1)	//To Go
		printf("\nCustomer %d receives token number %d from the OrderTaker %d\n", i, C[i].custTokenNo, j);
	else				//eat in
		printf("\nCustomer %d is given token number %d by the OrderTaker %d\n", i, C[i].custTokenNo, j);
	}
	

	tokenNo++;				//Token No incremented
	//Token No assigned and incremented
    	

	// *****		Token Part Ends		***** //


	//TO GO PART BEGINS ************** //
	
	if (C[i].orderType == 1)
	{

		waitToGoLock[i] -> Acquire();			//Customer C[i] acquires its lock for the Bagger - To Go Customer
	        baggerQueueLock-> Acquire(); 
 
		baggerQueue -> Append((void *) OT[j].otTokenNo);	//The Order which needs to be bagged,


		ITEMSINQ++;			
                baggerQueueLock->Release();     				//its Token No. is appended to the bagger queue



	
		
    		// *****		Monitor for TO GO Customers Begins	***** //




	if (C[i].toGoWaiting==0)
		C[i].toGoWaiting=1;				//Customer C[i]'s toGoWaiting flag is set now,
		



		orderTakerLock[j]->Release();		

			

		waitToGoCV[i] -> Wait(waitToGoLock[i]);		//Customer  C[i] waits for the order to be bagged and
			

	while(C[i].toGoWaiting == 1 && C[i].custDoA == 1) 

	{		
	

	
		for (x=0; x < noOrderTakers; x++)			//Checks each OrderTaker
		{
		
			if (OT[x].otBagTokenNo == C[i].custTokenNo && C[i].custDoA == 1 )	//Sees if the bagged order's token 													//no matches his own token
												//no
			{
				waitToGoCV[i] -> Signal(waitToGoLock[i]);

				C[i].toGoWaiting=0;
				C[i].custDoA = 0;
	

				OT[x].otBagTokenNo = -1;		//Token no matches. So OT[x]'s bagging flag is reset to -1
		
	
				waitToGoLock[i] -> Release();		//Can release the lock now as the customer has
 			
				return;					//Exits the orderProcessingfn()			
				}
			
			
		}
	
		if (C[i].custDoA == 1){

		waitToGoCV[i] -> Wait(waitToGoLock[i]);		//Customer  C[i] waits for the order to be bagged and
								//the order to be bagged and 
			}
								//the signal from the bagger	

}

	}								//if TO GO ends here

    		// *****		Monitor for TO GO Customers Ends	***** //

		// *****		Monitor for EAT IN Customers Begins	***** //

	else if(C[i].orderType==0)
	{       
                baggerQueueLock->Acquire();
		
                baggerQueue -> Append((void *) OT[j].otTokenNo);	//The Order which needs to be bagged,

                ITEMSINQ++;
		baggerQueueLock->Release();
                orderTakerLock[j] -> Release();		

		//Wait for acquiring the table
		
		waitEatInLock -> Acquire();		//Acquires lock for waiting on the waiter


		printf("\nCustomer %d is waiting for the waiter to serve the food\n", i);


		waitEatInCV -> Wait(waitEatInLock);	//Waiting for waiter's broadcast

		
		//Customer has received the broadcast and now is awake and has the waitEatInLock
				
		while (C[i].waiterAssigned == -1)
		{	
			for (x=0; x < noWaiters; x++)
			{
				if (W[x].wTokenNo == C[i].custTokenNo)		//Broadcast was for this customer's TokenNo
				{
					waitEatInCV -> Signal(waitEatInLock);	//Signals the waiter that the token no has matched
				
					C[i].waiterAssigned=x;
		
					printf("\nWaiter %d validates the token number for Customer %d\n",x,i);
					printf("\nWaiter %d serves food to Customer %d\n", x, i);
                                	printf("\nCustomer %d is served by waiter %d\n", i, x);		

					customerCounterLock -> Acquire();
					customerCounter++;
					customerCounterLock -> Release();

					break;	
				}
			}

			
			//Token didn't match
			
			waitEatInCV -> Wait(waitEatInLock);	//Waiting again for waiter's broadcast
		}
		
		
		//Waiter Assigned by now
	
		waitEatInLock -> Release();			//Acquires lock for waiting on the waiter


		while (true)
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
				printf("Waiting for table****************************");
				currentThread -> Yield();
			}
			else
				break;

		}

	
		//By the time the loop ends, C[i] has been assigned a table
		//Customer's Table ID has been set and Table's status is changed

		printf("\nCustomer %d is seated at table number %d\n", i, C[i].custTableID);	
							
	
		//The time taken by the customer on the table, once he reaches there
		
                
		for (x=0; x<1; x++)
			currentThread -> Yield();

		
		//Customer is done eating
		
		T[C[i].custTableID].tStatus=TFree;

		C[i].custDoA=0;
				
		
	}

	// *****		Monitor for EAT IN Customers Ends	***** //




}		



//Function for Bagging Food Process
								
void baggingFn(int j, int bagTokenNo)						//j is the index of the Bagger, i.e. OT[j]
{

	int x;

	for ( x=0; x<noCustomers; x++)
	
	//TO GO part
	if (C[x].toGoWaiting == 1 && C[x].custDoA == 1 && C[x].orderType==1)
	{	
		
		waitToGoLock[x] -> Acquire();			//Bagger acquires the lock of Customer C[i] for the Bagger -
		DEBUG('t', "The Bagger acquires the lock of Customer C[i] for the Bagger");	//informative message for debugging
	    	 
	}


	int i;							//For the index of the Customer, i.e. C[i]

	i=tokenToId[bagTokenNo];				//Obtained Customer ID from the Bag Token No
	

	OT[j].otBagTokenNo=bagTokenNo;
	

//	while (sixBurgerAvailable < C[i].sixBurger || threeBurgerAvailable < C[i].threeBurger || veggieBurgerAvailable < C[i].veggieBurger || frenchFriesAvailable < C[i].frenchFries) currentThread -> Yield();

	cookedLock -> Acquire();
	sixBurgerAvailable-=C[i].sixBurger;			//The amount of food item of each type available
	threeBurgerAvailable-=C[i].threeBurger;			//on its respective shelf is decremented by the number
	veggieBurgerAvailable-=C[i].veggieBurger;		//of items of that type ordered by the Customer C[i]
	frenchFriesAvailable-=C[i].frenchFries;
	cookedLock -> Release();	
	

	if (C[i].orderType == 1)
	{
	
    		// *****		Monitor for TO GO Customers Begins	***** //

		if (M.mID==j){
		printf("\nManager %d packed the food for Customer %d", j, i);
		printf("\nManager %d gives food to Customer %d", j, i);
		printf("\nCustomer %d receives food from the Manager %d\n", i, j);
		printf("\nCustomer %d is leaving the restaurant after Manager %d packed the food\n", i, j);

		customerCounterLock -> Acquire();
		customerCounter++;
		customerCounterLock -> Release();


		}
		else
		{
		printf("\nOrderTaker %d packed the food for Customer %d", j, i);
		printf("\nOrderTaker %d gives food to Customer %d", j, i);
		printf("\nCustomer %d receives food from the OrderTaker %d\n", i, j);
		printf("\nCustomer %d is leaving the restaurant after OrderTaker %d packed the food\n", i, j);

		customerCounterLock -> Acquire();
		customerCounter++;
		customerCounterLock -> Release();


		}


		if (C[i].toGoWaiting==1);
		{		DEBUG('t', "The Signal is given to the customer");	//informative message for debugging
	    	 
			waitToGoCV[i]->Signal(waitToGoLock[i]);			
		}
	
	for (x=0; x < noCustomers; x++)			//For each Customer
		{
			if(waitToGoLock[x]->isHeldByCurrentThread() && x!=i)
		{
			waitToGoLock[x] -> Release();
			}
		}
	


	waitToGoCV[i] -> Wait(waitToGoLock[i]);
	waitToGoLock[i] -> Release();

			
		return;							//Exits fn
	}


	

    		// *****		Monitor for TO GO Customers Ends	***** //

	
	else
	{
		//If the control reaches here, the order is Eat in type. Therefore, the Token No is appended to the waiterQueue
    		//For Eat In Customers
	
		
		printf("\nOrderTaker %d gives token number %d to Customer %d", j, C[i].custTokenNo, i);
		
               waitQueueLock -> Acquire();				//Ensures that only Bagger or Waiter use the queue at a time

		waiterQueue -> Append((void *) bagTokenNo);		//The Order which needs to be bagged,
									//its Token No. is appended to the waiter queue
		
		itemsInWaiterQueue++;
                
		waitQueueLock -> Release();
                

	}

	
	//return back to orderTakerFn	
						
}


//Manager goes to ATM Fn
void goToBankFn()
{
	for (int x=0; x < (rand() % 5) ; x++)
		currentThread -> Yield();
	
	printf("\nManager goes to bank to withdraw the cash\n");

	M.mCash+=CashWithdrawn;					//Manager gets richer by the CashWithdrawn Amount
}



//Manager Fn
//SET MANAGER'S ID WHEN USED AS AN ORDERTAKER

void managerFn(int p)
{
	int x;		//for loop index
	int seed;	//for generating random amount of wait time

	M.mCash=InitialManagerCash;
	M.mID=noOrderTakers ;

	//Manager as an OrderTaker Initialization

	
	int bagTokenNo;

	//OrderTaker OT[i] becomes active only when manager works as an ordertaker	


	int noCustomersServed=0;


	//MANAGER ENTERS SYSTEM

	while (true)
	{

		customerCounterLock -> Acquire();
		
		if (customerCounter == noCustomers)
			interrupt -> Halt();
					
		customerCounterLock -> Release();




			// ************MANAGER AS AN ORDERTAKER STARTS HERE***************
			
		
		if (custLineLengthMV > 3)		//More than 3 customers waiting
		{

			noOrderTakers++;	
		
					orderTakerLock[M.mID]=new Lock("OrderTaker_Lock");		//orderTakerLock Created
					orderTakerCV[M.mID]=new Condition("Order_Taker_CV");	//orderTakerCV Created

		
			OT[M.mID].otStatusMV=OTBusy;		
			OT[M.mID].otID=M.mID;                			//OrderTaker's ID - otID goes like 1, 2, 3, ....
   			OT[M.mID].otTokenNo=-1;				//Initialized to -1 as no Token No. given right now
			OT[M.mID].otCash=0.00;
		
			// **************START OF ORDERTAKER CODE*************
			
			

			while(noCustomersServed <= 2)           	
     			{							//OT works continuously, checking to see if he has  											//served 2 customers
    				custLineLock -> Acquire(); 			//OT acquires custLineLock so that he can 
										//later signal the waiting Customer

			     				
        			if (custLineLengthMV > 0)        		//i.e. someone's waiting in the line
        			{      
					custLineLengthMV--; 			//CustLineLength decremented as a 
										//customer has ended witing in line
					OT[M.mID].otStatusMV=OTWaiting;		//OT[i] is now in 'Waiting' state as it 
										//has already signaled 
					DEBUG('t', "The Customer signals the customer");	//informative message for debugging
                       			custWaitingCV -> Signal(custLineLock); 	//OT Signals Customer
	

        			}

       				else if (ITEMSINQ > 0)
        			{
					
					noCustomersServed++;

                    		    	baggerQueueLock->Acquire();			
            				bagTokenNo=(int)baggerQueue -> Remove();
					ITEMSINQ--;
                        		baggerQueueLock->Release(); 

			
	     				custLineLock -> Release();        	//Food is now bagged so the custLineLock is released
					DEBUG('t', "The Custlinelock is released");	//informative message for debugging
		
                       
					baggingFn(M.mID, bagTokenNo);		//The bagging function takes care of 		

		
            				continue;                		//OrderTaker will continously bag food if 
										//there are any food	
               	                 						//items that need to be bagged
        			}

     	  		
				
				orderTakerLock[M.mID] -> Acquire();    		//OrderTaker i should enter the Critical 
										//Region of Customer-Order
                     								//taker interaction before the Customer does
				if (custLineLock -> isHeldByCurrentThread())
    				custLineLock -> Release();    			//OrderTaker is now in the Customer-OrderTaker
										// Critical Region	
					    				       	//hence, the custLineLock is no longer required
			
				orderTakerCV[M.mID] -> Wait(orderTakerLock[M.mID]);	//OrderTaker i is waiting for Customer's 
 			
				noCustomersServed++;

				orderTakerLock[M.mID]->Release();    		//OrderTaker i releases the OrderTakerLock
	
			}                  					//while(true) loop ends here                
			

			noOrderTakers--;					//OT[i] is inactive now
			OT[M.mID].otStatusMV=OTFree;		

		}								//if condition ends here		
	

			// *Manager as OrderTaker ends here....... *
		 if(sixBurgerAvailable<=foodthreshold)
		{
			
			for(int i=0;i<noCooks;i++)
			{
				if(Coo[i].Status==0)
					{
						Coo[i].Status=1;
						printf("\nManager informs Cook %d to cook 6-dollar burger\n", Coo[i].cookid);
						printf("\nCook %d is going to cook 6-dollar burger\n", Coo[i].cookid);
						cookManagerLock->Acquire();
						cookMCV-> Signal(cookManagerLock);
						cookManagerLock->Release();
					}
				else if(Coo[i].Status==2)
					{	
						Coo[i].Status=1;
						printf("Cook %d returned from break", Coo[i].cookid);
						printf("\nManager informs Cook %d to cook 6-dollar burger\n", Coo[i].cookid);
						printf("\nCook %d is going to cook 6-dollar burger\n", Coo[i].cookid);
						cookManagerLock->Acquire();
						cookMCV-> Signal(cookManagerLock);
						cookManagerLock->Release();
									
					}
				else
					{	
						currentThread->Yield();
					}
			}      	
		}
	else 
		// compare the value with the specific amount in the inventory. make cook Sleep*/
		{
			for(int i=0;i<noCooks;i++)
			{
				if(Coo[i].Status==0)
				{
					Coo[i].Status==2;
					printf("Cook %d is going on break", Coo[i].cookid);
				}
			}
		}
		
			
		if(threeBurgerAvailable<=foodthreshold)
		{
		for(int i=0;i<noCooks;i++)
			{
					if(Coo[i].Status==0)
					{
						Coo[i].Status=1;
						printf("\nManager informs Cook %d to cook 3-dollar burger\n", Coo[i].cookid);
						printf("\nCook %d is going to cook 3-dollar burger\n", Coo[i].cookid);
						cookManagerLock->Acquire();
						cookMCV-> Signal(cookManagerLock);
						cookManagerLock->Release();
					}
					else if(Coo[i].Status==2)
					{
						Coo[i].Status=1;
						printf("Cook %d returned from break", Coo[i].cookid);
						printf("\nManager informs Cook %d to cook 3-dollar burger\n", Coo[i].cookid);
						printf("\nCook %d is going to cook 3-dollar burger\n", Coo[i].cookid);
						cookManagerLock->Acquire();
						cookMCV-> Signal(cookManagerLock);
						cookManagerLock->Release();
									
					}
				     else
					{
						currentThread->Yield();
					}
			}      	
		}
	
		else
		{
			for(int i=0;i<noCooks;i++)
			{
				if(Coo[i].Status==0)
				{
					Coo[i].Status==2;
					printf("Cook %d is going on break", Coo[i].cookid);
				}
			}
		}
			


		if(veggieBurgerAvailable<=foodthreshold)
		{
		for(int i=0;i<noCooks;i++)
			{
				if(Coo[i].Status==0)
					{
						Coo[i].Status=1;
						printf("\nManager informs Cook %d to cook veggie burger\n", Coo[i].cookid);
						printf("\nCook %d is going to cook veggie burger\n", Coo[i].cookid);
						cookManagerLock->Acquire();
						cookMCV-> Signal(cookManagerLock);
						cookManagerLock->Release();
					}
				else if(Coo[i].Status==2)
					{
						Coo[i].Status=1;
						printf("Cook %d returned from break", Coo[i].cookid);
						printf("\nManager informs Cook %d to cook veggie burger\n", Coo[i].cookid);
						printf("\nCook %d is going to cook veggie burger\n", Coo[i].cookid);
						cookManagerLock->Acquire();
						cookMCV-> Signal(cookManagerLock);
						cookManagerLock->Release();
									
					}
				else
					{
						currentThread->Yield();
					}
			}      	
		}
	
	else 
		{
			for(int i=0;i<noCooks;i++)
			{
				if(Coo[i].Status==0)
				{
					Coo[i].Status==2;
					printf("Cook %d is going on break", Coo[i].cookid);
				}
			}
		}
		
		if(frenchFriesAvailable<=foodthreshold)
		{
		for(int i=0;i<noCooks;i++)
			{
				if(Coo[i].Status==0)
					{
						Coo[i].Status=1;
						printf("\nManager informs Cook %d to cook french fries\n", Coo[i].cookid);
						printf("\nCook %d is going to cook french fries\n", Coo[i].cookid);						cookManagerLock->Acquire();
						cookMCV-> Signal(cookManagerLock);
						cookManagerLock->Release();
					}
				else if(Coo[i].Status==2)
					{
						Coo[i].Status=1;
						printf("Cook %d returned from break", Coo[i].cookid);
						printf("\nManager informs Cook %d to cook french fries\n", Coo[i].cookid);
						printf("\nCook %d is going to cook french fries\n", Coo[i].cookid);					cookManagerLock->Acquire();
						cookMCV-> Signal(cookManagerLock);
						cookManagerLock->Release();
									
					}
				else
					{
						currentThread->Yield();
					}
			}      	
		}
	else 
		// compaire the value with the specific amount in the inventory. make cook Sleep*/
		{
			for(int i=0;i<noCooks;i++)
			{
				if(Coo[i].Status==0)
				{
					Coo[i].Status==2;
					printf("Cook %d is going on break", Coo[i].cookid);
				}
			}
		}


			
		
		// ************MANAGER AS AN ORDERTAKER ENDS HERE***************



		
		// ***************MAINTAINING UNCOOKED STARTS*******************		
	 
		uncookedLock -> Acquire();		//accessing noUncooked variable
	
		if (noUncooked < UncookedThreshold)	//if current uncooked < threshold
		{
						
			//check if cash available after collecting each OT's cash, i.e. removes each of the OT's cash &
			//adds it to his own cash

			otCashLock -> Acquire();
			
			for (x=0; x < noOrderTakers; x++)
			{
				M.mCash+=OT[x].otCash;
				OT[x].otCash=0.00;
			}
	
			otCashLock -> Release();			
						
			// by this time, mCash has total cash in the system

			//cash reqd for this order=AddedQuantity * UncookedPrice

			while (noUncooked < UncookedThreshold)
			{
				
				if (M.mCash >  AddedQuantity * UncookedPrice )
				{
					uncookedLock -> Release();
					//enough cash to place an order
				
					//order placed now, so cash decremented
					M.mCash -=AddedQuantity * UncookedPrice;
			
					printf("\nManager refills the inventory\n");

					//Order placed !!
				
					//Time waiting for order to come
				
					while (true)				//seed is 1 or 2
						if (!(seed=rand() % 2))		//CAN TAKE MACRO IF TIME LATER !!!!
							break;


					for (x=1; x <= seed * 5 * 2 ; x++)
						currentThread -> Yield();
						
					//Wait is now over!!

					uncookedLock -> Acquire();
					noUncooked+=AddedQuantity;

					printf("\nInventory is loaded in the restaurant\n");
				
					if (uncookedLock -> isHeldByCurrentThread())		//NOT REQD but for backup
			uncookedLock -> Release();
				}

				else 
				{
					if (uncookedLock -> isHeldByCurrentThread())		//NOT REQD but for backup
			uncookedLock -> Release();
					//NO Cash
					//Needs to go to the ATM
					goToBankFn();
					uncookedLock -> Acquire();
					//Manager is now richer by CashWithdrawn
				}
			}						//while loop ends here
		}							//uncooked items ordered

		if (uncookedLock -> isHeldByCurrentThread())		//NOT REQD but for backup
			uncookedLock -> Release();
		

		// ***************MAINTAINING UNCOOKED ENDS*******************


		
		// ******************************Waiter Manager Interaction

		
		if(itemsInWaiterQueue > WaitingOrderThreshold)		
		{
			
			waiterLock -> Acquire();		//Manager Waiter interaction Lock	
	
			

			printf("\nManager calls back all Waiters from break\n");			

			waiterCV -> Signal(waiterLock);	
			
			waiterLock -> Release();
		}
	
		currentThread->Yield();
	}


}

	
void waiterFn(int i)
{

	W[i].wID=i;
	W[i].wTokenNo=-1;
	W[i].wStatus=WFree;
	
   waiterLock -> Acquire();		//Manager Waiter interaction Lock	
	waiterCV -> Wait(waiterLock);		//Waiter waits for the manager's signal, so that he can wake up
waiterLock -> Release();
	printf("\nWaiter %d enters the system", i);

//	while (true)
//	{
		while (itemsInWaiterQueue > 0)	//Pending Orders !
		{

			printf("\nWaiter W%d is attending to pending orders\n", i);
		
			waitQueueLock -> Acquire();	//Acquires the lock for accessing the waiterQueue
			
			W[i].wTokenNo=(int)waiterQueue -> Remove();	//The Order which needs to be served, is removed
	
			itemsInWaiterQueue--;		//Items in the Queue decremented

			waitQueueLock -> Release();

			

			waitEatInLock -> Acquire();	//Acquires the lock for broadcasting the customers that thir order is ready

			waitEatInCV -> Broadcast(waitEatInLock);	//Broadcasts for token wTokenNo

			waitEatInCV -> Wait(waitEatInLock);		//Waits for the customer whose token id matches

			if (waitEatInLock -> isHeldByCurrentThread())
 				waitEatInLock-> Release();
		}

		//Nothing to do

		printf("\n*******Waiter W%d is going on break*****\n", i);

		
		 waiterLock -> Acquire();		//Manager Waiter interaction Lock	
		waiterCV -> Wait(waiterLock);		//Waiter waits for the manager's signal, so that he can wake up
		waiterLock -> Release();
	


	

}
	

//Function for the Customer where 'i' is the Customer ID

void custFn(int i)
{

	int j;

	// *****			Initialization Part Begins		*****
    	//ith Customer's Structure variables are initialized. For information unavailable right now, -1 value is assigned
    
    	C[i].custID=i;                			//Customer's ID - custID goes like 1, 2, 3, ....
    	C[i].myOrderTaker=-1;        			//Initialized to -1 as no OrderTaker assigned right now
    	C[i].custTokenNo=-1;           			//Initialized to -1 as no Token No. given right now
 //   	C[i].orderStatus=0;				//Initially orderStatus is set to 0, i.e. Not Served Yet

	C[i].custTableID=-1;
	C[i].waiterAssigned=-1;
	C[i].toGoWaiting=0;
	C[i].custDoA=1;
	
	C[i].orderType=rand()%OrdertypeMAX;			//Order Type is randomly generated as Eat In or To Go using rand()
    	C[i].sixBurger=rand() % OrderQuantityMAX;      	//6$ Burger ordered or not?? 0 for No, else quantity is specified
 	C[i].threeBurger=rand() % OrderQuantityMAX;    	//3$ Burger ordered or not?? 0 for No, else quantity is specified
    	C[i].veggieBurger=rand() % OrderQuantityMAX;  	//Veggie Burger ordered or not?? 0 for No, else quantity is specified
    	C[i].frenchFries=rand() % OrderQuantityMAX;    	//French Fries ordered or not?? 0 for No, else quantity is specified
    	C[i].soda=0;
//rand() % OrderQuantityMAX;    	//Soda ordered or not?? 0 for No, else quantity is specified


	// *****			Initialization Part Ends		*****


    	//Customer enters the system. Acquires custLineLock and holds it till he reaches the OrderTaker

	custLineLock -> Acquire();        			//Customer acquires custLineLock
	DEBUG('t', "The Customer owns the custLineLock");	//informative message for debugging
    	
				//Customer Order Type displayed
	
	if (C[i].orderType==1)
		printf("\nCustomer %d chooses to to-go the food\n",i); 			
	else
		printf("\nCustomer %d chooses to eat-in the food\n",i); 
		
	
   
if(C[i].sixBurger >0)
{     	printf("\nCustomer %d is ordering 6-dollar burger\n",i);
}
if(C[i].sixBurger == 0)
{        printf("\nCustomer %d is not ordering 6-dollar burger\n",i);          
}
if(C[i].threeBurger >0)
{     	printf("\nCustomer %d is ordering 3-dollar burger\n",i);
}
if(C[i].threeBurger == 0)
{     	printf("\nCustomer %d is not ordering 3-dollar burger\n",i);
}
if(C[i].veggieBurger >0)
{     	printf("\nCustomer %d is ordering veggie burger\n",i);
}
if(C[i].veggieBurger == 0)
{     	printf("\nCustomer %d is not ordering veggie burger\n",i);
}      
if(C[i].frenchFries >0)
{     printf("\nCustomer %d is ordering french fries\n",i);
}
if(C[i].frenchFries ==0)
{     printf("\nCustomer %d is not ordering french fries\n",i);
}


for(j=0; j < noOrderTakers; j++)    			//For each OrderTaker
        	if(OT[j].otStatusMV == OTFree)    		//Customer checks if OT[j] is Free
        	{  
			OT[j].otStatusMV=OTBusy;    		//OT[j] was found Free & is now set to Busy
            		C[i].myOrderTaker=j;    		//OT[j] has now been assigned to the Customer
			break;    
        	}

	

    	if(C[i].myOrderTaker == -1)         			//no one's available, get in line    
    	{
		DEBUG('t', "The Customer is not assigned an Order Taker the Lock");	//informative message for debugging
   		
        	custLineLengthMV++; 				//Customer is now waiting in line, so custLineLengthMV
        	custWaitingCV -> Wait(custLineLock);		//wait for OrderTaker to be available
    	}

	
	//If Customer C[i] has reached here, it means that it was signaled by an OrderTaker.As any Customer can go 
	//to and OrderTaker in the 'Waiting' State, the Customer goes to the first OrderTaker that it finds 'waiting'

    	for(j=0; j < noOrderTakers; j++)    			//For each OrderTaker
    	{
        	if(OT[j].otStatusMV == OTWaiting)    		//note: if this condition fails, we know it must be a 
   		{						//Manager doing the order taking
	     		C[i].myOrderTaker = j;         		//jth OrderTaker is assigned to C[i] now
			DEBUG('t', "The Customer %d has been assigned to OrderTaker %d",C[i],C[i].myOrderTaker);	 									//informative message for debugging
           					    		
 			OT[j].otStatusMV = OTBusy;		//Now the OrderTaker is Busy
			break;


/*TEMP:No order bagging yet!! So, once OrderTaker has been assigned to the Customer, assumed that he has been served 		       	
			OT[j].otStatusMV=OTFree;  		REMOVE LATER*/


   		}     	
    	}


 	//By now the Customer is done waiting in the queue and ready to place the order
	
       	custLineLock -> Release();         			//As the Customer is no longer in the queue
	DEBUG('t', "The Customer Line Lock is released");	//informative message for debugging                				
	orderTakerLock[C[i].myOrderTaker] -> Acquire();    	//Customer is now ready to give his order
	DEBUG('t', "The Customer Acquires the ordertaker Lock");//informative message for debugging
  	orderTakerCV[C[i].myOrderTaker] -> Signal(orderTakerLock[C[i].myOrderTaker]);	//Customer C[i] signals its OrderTaker 

			orderProcessingFn(i);			//The order function for Customer C[i] is called	
								//Function takes care of order, payment, assignment of token
								//and appending the token no in the bagger queue part

}

						
//Function for the OrderTaker where 'i' is the OrderTaker ID

void orderTakerFn(int i)
{
	int bagTokenNo;

	// *****			Initialization Part Begins		*****



    	//With OrderTaker's Structure variables are initialized. For information unavailable right now, -1 value is assigned
    
    	OT[i].otID=i;                			//OrderTaker's ID - otID goes like 1, 2, 3, ....
   	OT[i].otTokenNo=-1;				//Initialized to -1 as no Token No. given right now
	OT[i].otCash=0.00;				//Initially the OrderTaker has no Cash


	// *****			Initialization Part Ends		*****


    	while(true)                			//OT works continuously, checking to see if there are customers
    	{
		custLineLock -> Acquire();    		//OT acquires custLineLock so that he can later signal the waiting Customer
     		DEBUG('t', "The OrderTaker Acquires the custlinelock/n");	//informative message for debugging
		//The task of attending to customers waiting in the queue is given priority over the task of bagging food by the
    		//OrderTaker

        	if (custLineLengthMV > 0)        		//i.e. someone's waiting in the line
        	{      
			custLineLengthMV--; 			//CustLineLength decremented as a customer has ended witing in line
			OT[i].otStatusMV=OTWaiting;		//OT[i] is now in 'Waiting' state as it has already signaled 
			DEBUG('t', "The Customer signals the customer");	//informative message for debugging
                        custWaitingCV -> Signal(custLineLock); 	//OT Signals Customer
	

        	}

        	else if (ITEMSINQ > 0)
        	{

                        baggerQueueLock->Acquire();			
            		bagTokenNo=(int)baggerQueue -> Remove();
			ITEMSINQ--;
                        baggerQueueLock->Release(); 

			
	     		custLineLock -> Release();        	//Food is now bagged so the custLineLock Lock is released
			DEBUG('t', "The Custlinelock is released");	//informative message for debugging
		
                       
			baggingFn(i, bagTokenNo);		//The bagging function takes care of 		

		
            		continue;                		//OrderTaker will continously bag food if there are any food
               	                 				//items that need to be bagged
        	}

        	else                			        //There's no one waiting in the line nor is there any food left to 			
		{						//be bagged
        	
       			OT[i].otStatusMV=OTFree;    		//OT has nothing to do now. So sets his status to Free
        	}
    	  

	orderTakerLock[i] -> Acquire();    		//OrderTaker i should enter the Critical Region of Customer-Order
                     					//taker interaction before the Customer does
	DEBUG('t', "The OrderTaker acquires the  ordertakerLock");	//informative message for debugging
    	custLineLock -> Release();    			//OrderTaker is now in the Customer-OrderTaker Critical Region	
					           	//hence, the custLineLock is no longer required
	DEBUG('t', "The OrderTaker releases the customer");	//informative message for debugging
	orderTakerCV[i] -> Wait(orderTakerLock[i]);	//OrderTaker i is waiting for Customer's signal 
     

	orderTakerLock[i]->Release();    		//OrderTaker i releases the OrderTakerLock
	DEBUG('t', "The OrderTaker releases the lock");	//informative message for debugging
	}                  				//while(true) loop ends here                
      
}        


void CarlSimulation()
{
	
	int i;
	
	// *****			Initialization Part Begins		*****

		

	//New Locks created

	customerCounterLock=new Lock("Lock");

	cookManagerLock=new Lock("Cook_Manager");
	cookMCV=new Condition("Cook_Manager_Waiting");


	uncookedLock=new Lock("Uncooked_Lock");
	cookedLock=new Lock("cooked_Lock");
	otCashLock=new Lock("otcash_Lock");

	custLineLock=new Lock("Cust_Line_Lock");
	custWaitingCV=new Condition("Cust_Waiting_CV");   
  
	waiterLock=new Lock("Waiter_Manager_Lock");
	waiterCV=new Condition("Waiter_Manager_CV");

	waitEatInLock=new Lock("Waiter_EatIn_Lock");
	waitEatInCV=new Condition("Waiter_EatIn_CV");


	waitQueueLock=new Lock("Wait_Queue_Lock");
        baggerQueueLock= new Lock("Bagger_Queue_Lock");
	
     	Thread *t;
	

	baggerQueue=new List;
	waiterQueue=new List;
	

	for (i=0; i < CustomerMAX; i++)
		tokenToId[i]=-1;					//Initially no tokens in the system, so initialized
									//to -1	


	// *****			Initialization Part Ends		*****


	printf("\n\n*****Welcome to Carl Junior Simulation*****\n");
    
	printf("\n1.\tCustomers who wants to eat-in, must wait if the restaurant is full");
	printf("\n2.\tOrderTaker/Manager gives order number to the Customer when the food is not ready");
	printf("\n3.\tCustomers who have chosen to eat-in, must leave after they have their food and Customers who have chosen to-go, must leave the restaurant after the OrderTaker/Manager has given the food");
	printf("\n4.\tManager maintains the track of food inventory. Inventory is refilled when it goes below order level");
	printf("\n5.\tA Customer who orders only soda need not wait");
	printf("\n6.\tThe OrderTaker and the Manager both sometimes bag the food");
	printf("\n7.\tManager goes to the bank for cash when inventory is to be refilled and there is no cash in the restaurant");
	printf("\n8.\tCooks goes on break when informed by manager");
	printf("\n9.\tRun the Entire Simulation\n\n\n");
	
	printf("\nEnter the choice: ");
	
	scanf("%d", &choice);


	if (choice == 1)
	{
		noTables=4;
	}



	if (choice == 2)
	{
		sixBurgerAvailable=0;
		threeBurgerAvailable=0;
		veggieBurgerAvailable=0;
		frenchFriesAvailable=0;
	}

	if (choice == 3)
	{
	;//do nothing, already works in routine simulation
	}
	
	if (choice == 4)
	{
		UncookedStartingAmount=0;		
	}	
	

	if (choice == 6)
	{
		noOrderTakers=1;
	}

	if (choice == 7)
	{
		UncookedStartingAmount=0;
		InitialManagerCash=0.00;		
	}
	
	if (choice != 6)
	{
	do
	{
		printf("\nEnter the number of order takers\n");
        	scanf("%d",&noOrderTakers);
	} while (noOrderTakers < 3);					
  	}
	do
	{
        	printf("\nEnter the number of customers\n");
        	scanf("%d",&noCustomers); 
	} while (noCustomers < 5);
  
	do
	{
		printf("\nEnter the number of waiters\n");
        	scanf("%d",&noWaiters); 
	} while (noWaiters < 3);

	
	if (choice != 1)
	{	do
		{
			printf("\nEnter the number of tables\n");
        		scanf("%d",&noTables); 
		} while (noTables < 1);
	}
	do
	{
		printf("\nEnter the number of Cooks\n");
        	scanf("%d",&noCooks); 
	} while (noCooks < 4);



  	printf("\nNumber of OrderTakers=%d\n",noOrderTakers); 
   	printf("\nNumber of Waiters=%d\n",noWaiters); 
	printf("\nNumber of Cooks=%d\n",noCooks); 
   	printf("\nNumber of Customers=%d\n",noCustomers); 
  	
	printf("\nTotal Number of tables in the Restaurant=%d\n",noTables); 

	printf("\nMinimum number of cooked 6-dollar burger=%d\n", CookedThreshold); 
  	printf("\nMinimum number of cooked 3-dollar burger=%d\n", CookedThreshold); 
  	printf("\nMinimum number of cooked veggie burger=%d\n", CookedThreshold); 
  	printf("\nMinimum number of cooked french fries burger=%d\n", CookedThreshold); 

for (i=0; i < noTables; i++)
	{
		T[i].tID=i;
		T[i].tStatus=TFree;
	}

for (i=0; i < noCooks; i++)
	{
		t=new Thread("Cook");
		Coo[i].Status= 0;		//O for free and 1 for busy
		t->Fork((VoidFunctionPtr)cookFn, i);
	}

 for (i=0; i < noWaiters; i++)
	{
		t=new Thread("Waiter");
		t->Fork((VoidFunctionPtr)waiterFn, i);
	}
    	
    
   	
    	for (i=0; i < noOrderTakers; i++)
    	{
		orderTakerLock[i]=new Lock("OrderTaker_Lock");		//orderTakerLock Created
		orderTakerCV[i]=new Condition("Order_Taker_CV");	//orderTakerCV Created

	
	  	t = new Thread("OrderTaker");         			//New thread for OrderTaker

   		OT[i].otStatusMV=OTBusy;          			//Initially OT[i]'s status is set to Busy
		OT[i].otBagTokenNo=-1;					//Initially OT[i]'s token no, which's being bagged
									//right on is set to -1
 	
	       	t->Fork((VoidFunctionPtr)orderTakerFn,i);		//Thread for OrderTaker forked, i is the OrderTaker's ID
    	}
    

	
    	

    	for (i=0; i < noCustomers; i++)
    	{
		waitToGoLock[i]=new Lock("waitToGo_Lock");			//waitToGoLock Created
		waitToGoCV[i]=new Condition("waitToGo_CV");			//waitToGoCV Created

		
        	t = new Thread("Customer");      	  		//New thread for Customer 
           	t->Fork((VoidFunctionPtr)custFn,i);  			//Thread for Customer forked, i is the Customer's ID    
    	}

       int m=1;
	t = new Thread("Manager");         
		
       	t->Fork((VoidFunctionPtr)managerFn,m);	



}

#endif
