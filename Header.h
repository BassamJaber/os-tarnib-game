#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <string.h>
#include <signal.h>

//*structures and Prototype for Card structures*//
typedef struct node *ptrl;
typedef struct node Arr;

struct node {
  int cardNum;
  int type;
  int state;
  ptrl next; 
};

/*Consider the Type As Following
 * Spades =1;
 * Hearts =2;
 * dimonds =3;
 * clubs =4;
 */

/* consider teh Card Numbers from 2 to 10 
 * J = 11 , Q = 12 , K = 13, A = 14;
 * Make a Value Propotional to it's Real Value
 */

typedef ptrl List;
typedef ptrl Position;

void insert(List,Position);
int isEmpty(List);
List createList(void);
void initializeCards(List L);
void printCardSet(List L);
List shuffle(List L);

struct call 
{
   int callNum;
   int TarnibType;
};
	
typedef struct sharedMemory *ptr;


struct sharedMemory{
   Arr cardSet[52];              /*Card Set , Shuffled and Saved in Shared Memory*/
   int shuffledPlayer;           /*the Number of Player who shuffled the Cards Last Round*/

   Arr player1[13];              /*Cards For Player Number One , After Distribution from Process 0 */
   Arr player2[13];              /* Cards For Player Number Two */
   Arr player3[13];              /* Cards For Player number three */         
   Arr player4[13];              /* Cards For Player Number Four */

   struct call Calls[4];          /* -1 indicate Pass , and Calls must be between 7-13 */
   int flag;                      /*this flag is used to make sure that all 4 Player done there Work before Returning to Parent*/
 
   
   int maxCall;                   /* indicate the maximum Call for Specific Player use to compute Result at the end */ 
   int PlayerNum;                 /* the number of the Player that Should Start Playing , Highest Call */
   int Tarnib;                    /* this will indicate the Tarnib type each Round */
   int TeamNum;
    	
   int Category;                  /* indicates the type of Cards Played */
   Arr PlayedRoundCards[4];       /*this array will contain Four Cards that will be played each round such that the Parent with Decide 
                                    which Player or Team won */
  
   int TeamOne;                   /* this will contain the Score for the Team During Playing the Hand */
   int TeamTwo;                   /* Cintain Score for Team Two */
   
   int TotalOneScore;            /* Total Score for Team One */
   int TotalTwoScore;             /* Total Score for Team Two */

   int Determine;                /*this Variable will Save the Name of Player who Won a sub round*/
   
};

typedef ptr Pointer;

union semun {
  int              val;
  struct semid_ds *buf;
  ushort          *array; 
};

struct sembuf acquire = {0, -1, SEM_UNDO}, 
              release = {0,  1, SEM_UNDO};

int countRounds;
 int DetermineWinner();
 void StoreShuffuledToSharedMemory(Pointer P , List Shuffled);
 void printSharedSet(Pointer sharedMemory);
 void DistributeCards(Pointer P , int PlayerNumber );
 int  getNumOfSubRounds(Pointer P , int PlayerNumber, int * Tar);
void toLinkedList(List L , int playerNum,Pointer sharedMemory);
void SelectCard(List L , Pointer sharedMemory, int PlayerNum);
void Selectfirst(Pointer sharedMemory , int PlayerNum);
void Selectfirst(Pointer sharedMemory , int PlayerNum);
void chooseCard(Pointer sharedMemory, int PlayerNum);
int isUsed(Pointer sharedMemory, int PlayerNum);
