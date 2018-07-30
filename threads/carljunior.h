
enum orderTakerStatus {OTFree, OTBusy, OTWaiting};	//Enumeration for the Status of OrderTaker
enum tableStatus {TFree, TBusy};
enum waiterStatus {WFree, WBusy};
//Structure for the Customer

typedef struct Cust	
{

  	int custID;		//customer id for its identification
 	int myOrderTaker;	//denotes the index of the OrderTaker assigned to the customer
	int custTokenNo;	//denotes the Token No. given to the Customer by the OrderTaker
	int orderType;		//orderType denotes the type of Customer's Order 0 for Eat In & 1 for To Go 
	
	int toGoWaiting;	//denotes if the Customer is waiting in the To Go Monitor for the tokens to be 
				//matched by the bagger. Initialized to 0 if not waiting, else 1
	
	int custDoA;		//denotes if the Customer is Dead or Alive
				//Initialized to 0 if Customer's Dead, 1 if Customer's Alive

	int custTableID;	//initialized to -1
	int waiterAssigned;	//initialized to -1


	//Food items available at the Restaurant. Set to the quantity of items ordered,if ordered else 0

	int sixBurger;		// 6$ Burger
	int threeBurger;	// 3$ Burger
	int veggieBurger;	// Veggie Burger
	int frenchFries;	// French Fries
	int soda;		// Soda

}Cust;


typedef struct OrderTaker
{

	int otID;		//OrderTaker id for its identification
	int otTokenNo;		//OrderTaker stores the Token No. assigned to the Customer

	int otBagTokenNo;	//When the OrderTaker acts as a bagger, the Token No he bags is stored here
				//Value initialized to -1		

	double otCash;		//Denotes the amount of cash available with the OrderTaker
				//initialized to 0.00

	orderTakerStatus otStatusMV;	//Denotes if the OrderTaker is Free, Busy or Waiting 
	
}OrderTaker;

typedef struct Waiter
{	
	int wID;
	int wTokenNo;
	int wStatus;
}Waiter;

typedef struct Tab
{
	int tID;
	int tStatus;

	//nt seatedCustID;	
}T1;

typedef struct Manager
{
	int mID;
	double mCash;
}Manager;

typedef struct Cook
{
	int cookid;
	int Status;
}Cook;






