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


	
void main()
{
	int i;

	Acquire(waiterIDGeneratorLock);
	i=waiterIDGenerator++;		
	Release(waiterIDGeneratorLock);



	W[i].wID=i;
	W[i].wTokenNo=-1;
	W[i].wStatus=WFree;
	

    	Acquire(waiterLock);		/*Manager Waiter interaction Lock */	
	 Wait(waiterCV,waiterLock);		/*Waiter waits for the manager's signal, so that he can wake up */
 	Release(waiterLock);
	Print("\nWaiter %d enters the system", i,0,0);

/*	while (true)*/
/*	{ */
		while (itemsInWaiterQueue > 0)	/*Pending Orders ! */
		{

			Print("\nWaiter W%d is attending to pending orders\n", i,0,0);
		
			 Acquire(waitQueueLock);	/*Acquires the lock for accessing the waiterQueue */
			
			W[i].wTokenNo=RemoveList(waiterQueue);	/*The Order which needs to be served, is removed */
	
			itemsInWaiterQueue--;		/*Items in the Queue decremented */

			Release(waitQueueLock);

			

			 Acquire(waitEatInLock);	/*Acquires the lock for broadcasting the customers that thir order is ready*/

			  Broadcast(waitEatInCV,waitEatInLock);	/*Broadcasts for token wTokenNo*/

			  Wait(waitEatInCV,waitEatInLock);		/*Waits for the customer whose token id matches*/

		/*	if (waitEatInLock -> isHeldByCurrentThread())	*/
 				Release(waitEatInLock);
		}

		/*Nothing to do*/

		Print("\nWaiter W%d is going on break\n", i,0,0);

		
		 Acquire(waiterLock);		/*Manager Waiter interaction Lock*/	
		 Wait(waiterCV,waiterLock);		/*Waiter waits for the manager's signal, so that he can wake up*/
		Release(waiterLock);
	
		Exit(0);
}


