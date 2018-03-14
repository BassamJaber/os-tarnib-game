#include "header.h"

int  getNumOfSubRounds(Pointer P , int PlayerNumber , int * Tar)
 {
   /*Number of Winning Sub round will be considered as follow
     * at first we see how many cards of each Type the player have , the best Color for tarnib 
     * then we count the number of cards that have values more than 11 = J , which considered as Possible 
     * winning Cards expect the Cards from Tarnib Type . and this give a good approximation for the winnng sub hands
     */
   if(P==NULL)
    {
      perror("Shared Memory");
      exit(0);
    }else{
          int count[4];   /*this array will count how many cards of each type the player have */
          int i ;
          for( i=0;i<4;i++)
               count[i]=0;          /*initialize the counter*/
           /*Determine which player want to calculate his winning rounds , By his Number*/
           i=0;
 	   switch(PlayerNumber)
                {
                  case 1: while(i<13) {
                             count[P->player1[i].type-1]++;
                             i++;
                            }
                          //printf("Player # 1\n");
			  break;
  	   	  case 2: while(i<13) {
                             count[P->player2[i].type-1]++;
                             i++;
                            }
                         // printf("Player # 2\n");
			  break; 
		  case 3: while(i<13) {
                             count[P->player3[i].type-1]++;
                             i++;
                            }
                         // printf("Player # 3\n");
			  break;
		  case 4: while(i<13) {
                             count[P->player4[i].type-1]++;
                             i++;
                            }
                         // printf("Player # 4\n");
			  break;
                }
       /* number of cards from each category
            for(i=0;i<4;i++)
                printf("%d \t ",count[i]);
            printf("\n");
      */
         int max=count[0], maxType=1;
         for(i=1;i<4;i++)
            {
  		if(count[i]>max)
                 {
                    max=count[i];
                    maxType=i+1;
                 }
            }

        // printf("Maximum Card Type is %d , Have %d Cards \n", maxType,max);
         *Tar=maxType;
         int winHand=max;
         /*we count the number of Tarnib Cards and the Possible Tarnib to use*/
         /*now we will count the number of Hight cards of other Types*/
            i=0;
            switch(PlayerNumber)
                {
                  case 1: while(i<13) {
                      
                             if( P->player1[i].type != maxType && P->player1[i].cardNum >11) 
                                 winHand++;
                             i++;
                            }
			  break;
  	   	  case 2: while(i<13) {
                             if( P->player2[i].type != maxType && P->player2[i].cardNum >11) 
                                 winHand++;
                             i++;
                            }
			  break; 
		  case 3: while(i<13) {
                             if( P->player3[i].type != maxType && P->player3[i].cardNum >11) 
                                 winHand++;
                             i++;
                            }
			  break;
		  case 4: while(i<13) {
                             if( P->player4[i].type != maxType && P->player4[i].cardNum >11) 
                                 winHand++;
                             i++;
                            }
			  break;
                }
         return winHand;
         }/*end of Function*/

 }
 void DistributeCards(Pointer P , int PlayerNumber )
 {
   /*we Have four Players in this game , when one player is Selected to start Distributing the Cards 
    *the cards are shuffled and stored in Shared Memory , now we will give Each player 13 Cards Depending on the 
    *Distribution Start Index .
    */    
   if(P==NULL)
   {
     perror("Shared Memory");
     exit(0);
   }else {
  	   int i=0; 
           int j=0; 
           int counter=0;
           int num=PlayerNumber;
           while(counter<4)
 	    {
              /*we will start distributed from the Player number +1  if player numebr 4 then next player is 1 */
              (num==4)?num=1:num++;
               switch(num)
                {
                  case 1: while(j<13) {
                             P->player1[j].type=P->cardSet[i].type;
                             P->player1[j].cardNum=P->cardSet[i].cardNum;
                             j++;
                             i++;
                            }
			  break;
  	   	  case 2: while(j<13) {
                             P->player2[j].type=P->cardSet[i].type;
                             P->player2[j].cardNum=P->cardSet[i].cardNum;
                             j++;
                             i++;
                            }
			  break; 
		  case 3: while(j<13) {
                             P->player3[j].type=P->cardSet[i].type;
                             P->player3[j].cardNum=P->cardSet[i].cardNum;
                             j++;
                             i++;
                            }
			  break;
		  case 4: while(j<13) {
                             P->player4[j].type=P->cardSet[i].type;
                             P->player4[j].cardNum=P->cardSet[i].cardNum;
                             j++;
                             i++;
                            }
			  break;
                }
             j=0;
             counter++;
           }
         }
 }
  
 void StoreShuffuledToSharedMemory(Pointer P , List Shuffled)
 {
   if(P==NULL)
   {
     perror("Shared Memory Pointer Invalid");
     exit(0);
   }else if(Shuffled==NULL)
          {
	    perror("List does Not exist");
            exit(0);
           } else {
 	           /*we have a pointer of shared Memory
            	    *we want to store the Shuffled Set of Cards into the Shared Memory
            	    *Static array that contain the set of 52 Card .
  	     	    */
                   int i=0;
                   Position Po=Shuffled->next;
    		   while(P!=NULL && i < 52)
                     {
		        P->cardSet[i].type=Po->type;
                        P->cardSet[i].cardNum=Po->cardNum;
                        Po=Po->next;
                        i++;
                     }
                  }
 }

 int DetermineWinner(){
   return (rand()%4);
 }
 List shuffle(List L)
 { 
   if(L==NULL)
    {
      perror("List Does Not Exist");
      exit(0);
    }else {
   	    int numOfCards=52;
            Position temp,P;
            List shuffled=createList();
            int random, index=0;

            while(numOfCards>0)
            {
              /* Select a Random Card */
	      random=rand()%numOfCards;
              
              P=L->next;
              temp=L;
              /* Get the random Card out of the List to shuffled List*/
              while(P!=NULL && random !=index)
                {
                  index++;
                  P=P->next;
                  temp=temp->next;
                }
                 index=0;	
                /*when this loop finish the Selected Card will be Pointed by P*/
                temp->next=P->next;     /*Remove it from List */
                insert(shuffled,P);     /* insert the Card to shuffled List */
                numOfCards--;           /*decrease the number of Cards Remaining to shuffle */
            }
           /*we USed Linked List to shuffle Cards to handle Them Easily , We Can move the Linked List to
            * an array to Store it in Shared Memory.
            */

          }
 }
 
 void printCardSet(List L)
 {
  if(L==NULL)
   {
     perror("Lsit does Not Exist");
     exit(0);
   }else {
 	   Position P ; 
  	   P=L->next;
           while(P!=NULL)
             {
                 printf("Card type %d , Card Number %d \n",P->type,P->cardNum);
                 P=P->next;
             }
         }
 }

 void initializeCards(List L)
 {
   if(L==NULL)
    {
      perror("Lsit Does not Exist");
      exit(0);
    }else{
            int i ,j;
 	    Position P;
            for(i=1;i<5;i++)
               for(j=2;j<15;j++)
                 {
 		    P=(Position)malloc(sizeof(struct node));
                    if(P==NULL)
                      {
			perror("Memory Full");
 			exit(0);
                      }else 	{	
                              P->cardNum=j;
  	               	      P->type=i;
 			      P->next=NULL;
			    }
                      insert(L,P);
                 } 
         }
 }

 void printSharedSet(Pointer sharedMemory)
 {
  int i;
  for(i=0;i<52;i++)
    {
      printf(" %d ",sharedMemory->cardSet[i].type);
      printf(" %d\n",sharedMemory->cardSet[i].cardNum);
   }
  
 }
void insert(List L , Position P)
{
  if(L==NULL)
   {
     perror("List Does Not Exist\n");
     exit(0);
   }else{
           /*Insert node at the first of the Linked List*/
           P->next = L->next;
           L->next=P;
        }
}

int isEmpty(List L)
{
  if(L==NULL)
   {
      perror("List Does Not Exist");
      exit(0);
   }else if (L->next != NULL)
         {
           return 0;
         }else {
  		   return 1;
               }
}

List createList()
{
 List L =(List)malloc(sizeof(struct node));
 if( L==NULL)
  {
    perror("Memory Full ");
    exit(0);
  }else{
         L->next= NULL;
         return L;
       }
}
