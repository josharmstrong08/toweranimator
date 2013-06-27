/* dynamicLights.h
	Written by: Jordan Lynn 
	email: snake.river52@gmail.com
	
	Light bars each get assigned their own node and are 
	given some type of identity, currently just a number, 
	for the structure. As well as three interger variables
	representing red, green and blue brightness for each
	LED.

	Currently each bar is represented by a node in a single linked
	list, there is only a head pointer currently but a tail could be
	added if it's needed for some reason.
*/
#include <stdio.h>
#include <stdlib.h>


struct lightBar{
	
	int red;
	int green;
	int blue;
	
	struct lightBar *next;
};

// Globals:
// root will be global for now
struct lightBar *root = NULL;

// Prototypes
void addToLL( struct lightBar *ptr );
void newLightBar();
void shutDown();
void printList();

int main(){
	int i;
	// Add in some switch for when the zigbee network detects a new
	// device entering the network.
	
	// Simulated scan and new device is found.
	for( i = 0; i < 5; i++ ){
		newLightBar();
		printf("light bar created!\n");
	}
	printf("printing list\n");	
	printList();	
		
	
return 0;
}

// Simple function to print the current LL
void printList(){
	struct lightBar *curr; // Conductor to traverse list

	curr = root;
	while( curr->next != NULL ){
		printf("current value %d\n", curr->red);
		curr = curr->next;	
	}
}

/* Currently this is only a linked list but the way I've built it the code can easily
	be adapted for other structures, it completely makes the new node so it can exist on
	it's own and THEN places it in the list which can be useful if we need to hurry and
	push the new node's information out onto the network for whatever reason. 

	Calling "newLightBar()" will create a new node with the light bar's info, it then
	could possibly be put into a linked list and a Binary Search Tree per say. This is
	why I usually build my data structures like this.

*/
void newLightBar(){
	// Ued to traverse list
	struct lightBar *curr;

	// Allocate new bar
	struct lightBar *ptr = (struct lightBar*)malloc(sizeof(struct lightBar));
	
	// Initialize it's values so that the lights don't come on
	// when it enters the network. 
	ptr->red = 0;
	ptr->green = 0;
	ptr->blue = 0;
	ptr->next = NULL;
	
	/* From here you can now push the new lightbar's information to it
	  out onto the network and it will be accurate so you could call a
	  function here to do so and it would update nodes outside of the LL.
	  I've always liked building structures like this because now we have
	  a completed mini node that can be stuck anywhere we like.
	*/
	// Now call the function to attach it to the end of the LL
	addToLL( ptr );
	// addToBST( ptr ); // example on how versitile this technique is
	// addToQueue( ptr ); // you can pass this node to any function that
						  // creates a data structure.
}

/*	Quick function that adds new node that is passed in to the
	 end of the LL if one doesn't exist yet the funciton will
	 create one.
*/
void addToLL( struct lightBar *ptr ){
	struct lightBar *curr; // Used to traverse the list.
	
	// Handle empty case, or rather this is the first device
	if ( root == NULL )
		root = ptr;
	
	else{
		// start at the start of course!
		curr = root;
		// now advance down the list until you reach the last node.
		while(	curr->next != NULL ){
			curr = curr->next;
		}
		// We should be at the end of the list now so
		// we can attach "ptr" the node made in the begining.
		curr->next = ptr;
	}

	// If we run out of memory we end up here
	if ( curr == NULL )
		printf("RAN INTO NULL IN \"newLightBar\" function!");
}

/* shutDown is an example of the new dynamic code working, it will
	 shut off all light bars in the network.
*/
void shutDown(){
	// curr is used to traverse and shut down LL
	struct lightBar *curr = root; // start at the begining of list 
	while( curr->next != NULL ){
		curr->red = 0; // set all colors to the 'off' position.
		curr->green = 0;
		curr->blue = 0;
		curr = curr->next; // advance to next node.
	}
	// All lights should be turned off! 
	// ... but I have a feeling I forgot something...
}
