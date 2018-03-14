#include "list.c"

int main(int argc , char * argv[])
{	
  int i , status;			
  pid_t pid, pid_array[4];    //*We have 4 Player Each Represented as Process , and Main Process Handle Them *//
  //*Temporary Variable that will hold the current Id of the Process , and an Array of Ids of Players Processes*//

  /*Create, attach and initialize the memory segment*/
  int shmid;        //*Shared Memory ID //
  pid=getpid();
  
  //*Allocate Shared Memory*//
  shmid = shmget((int) pid, sizeof(struct sharedMemory),IPC_CREAT | 0600);
  if(shmid==-1){
    perror("Shared Memory Failure");
  }else {
    	  printf("Shared Memory created \n");
        }

  //*Creating Structre For shared Memory and Attached it to Parent Process*//
  Pointer sharedMemory = (Pointer)shmat(shmid, 0, 0);

  //*Creating Semaphore Set*//
  //*Semaphore Set will Contain 5 Semaphores ,  one Semaphore For Each Player and One For Parent Process to Organize Work*//
 
  int semid;                 //*semaphore ID*//
  union semun    arg;
  static ushort  start_val[2]= {1,0};
  
  semid = semget((int) pid,5, IPC_CREAT | 0666);

  if(semid==-1){
     perror("Semaphore Fauilure");
   }else {
            printf("Semaphore Set Created Sucessfully\n");
         }
   //*creating semaphore is not Atomic , we must initialize the semaphores now *//
   arg.array = start_val;
   //*initialize all Semaphore Set to 0 *//
    if ( semctl(semid,0, SETALL, arg) == -1 ) {
      perror("semctl -- parent -- initialization");
      exit(3);
    }

while(sharedMemory->TotalOneScore != 61 && sharedMemory->TotalTwoScore !=61){
   //*Get Semaphore Number 0 for the Parent Process *//
    arg.val=1;
    semctl(semid,0,SETVAL,arg);   //*set the Value of Semaphore 0 to 1 , it can be Taken when calling Acquire*//
    
    //*Acquire Semaphore number 0 from Semaphore Set of the Val equal to 1 *//
    //* if val equal to zero Process Blocked *//

    acquire.sem_num =0;   
 
    if ( semop(semid, &acquire, 1) == -1 ) {
      perror("semop -- producer -- acquire");
      exit(4);
     }
    //*Critical Path Here*//
    /* Testing storing Cards Set in Shared Memory ( Working )*/
    List L = createList();
    initializeCards(L);
    List shuffled=shuffle(L); 			         
    StoreShuffuledToSharedMemory(sharedMemory,shuffled);
    DistributeCards(sharedMemory,1);
   // printSharedSet(sharedMemory);

    sharedMemory->shuffledPlayer=1;

     //*critical Part ends*//
    release.sem_num = 0;    //*Relase the Semaphore number 0 Critical Path ends *//

   //*creating four Processes and Save there IDs in Parent Process array *//
   //* fork Return 0 in Child , And Children ID for the Parent , So that the Parent Process Knows his Children ID's*//

  char str[15];//*Send the Player Number to the Processs , USed to Synchronize Processes with Sempahores *//
  for ( i = 0; i < 4; i++ ) {
    pid = fork();
    
    if ( pid == 0 ) {
      //printf("Player Ready\n");      
      sprintf(str, "%d", (i+1));   //*Convert the Integer Value for Player Number ToString *//
      execl("./child","child",str, (char *) NULL);
      perror("exec failure ");
      exit(0);
    }
    else {
      //*getPid() get the current running process that calling the function (System Call )*//
      pid_array[i] = pid;
      //* save all Players ID *//
    }
  }

    //*Give Permission for other Process to Start Working individually*//   
    //* Set Value for Semaphore Number 1 to 1  , When process (Player 1 ) try to Acquire it he could *// 
    arg.val=1;  
    //sharedMemory->shuffledPlayer          
    semctl(semid,1,SETVAL,arg);   

    sharedMemory->flag=1;    /*Always Reset the Flaf before Start Playing */   
    /*give Semaphore for Player 1 or in this Case we will give semaphores to Highest Call*/ 
    /*at the beginig first player should Shuffle the Cards 
     * then each player should see how many sub round the Could win and Update that in Shared Memory
     * then Parent should Moniter that every thing work ( last Player Give Semaphore to Parent to Moniter Work
     */ 

    //*Parent Freeze until Last Process Update the Number of Calls*//
    acquire.sem_num =0; 
 
    /*Semaphore Acquired , Test if All players put there Calls 
      * parent will decide which Player will start by giving him a semaphore
      */
    if ( semop(semid, &acquire, 1) == -1 ) {
      perror("semop -- producer -- acquire");
      exit(4);
     }
  /*  test each player Calls
      for(i=0;i<4;i++)
       { 
         printf("Player number %d  Calls %d ",(i+1),sharedMemory->Calls[i].callNum);
         printf("Tarnib type %d \n",sharedMemory->Calls[i].TarnibType);
       }
*/
    /*now after all players Calls , Parent should Decide who will Decide the Tarnib Category and give him Sempahore To Start Playing
     *
     */
     /*find the player number with maximum call , Set Player number and Tarnib Type and give him semaphore to start playing*/

     i=0 ;
     int max=sharedMemory->Calls[i].callNum;
     sharedMemory->Tarnib=sharedMemory->Calls[i].TarnibType;
     sharedMemory->PlayerNum=(i+1);
     for(i=1;i<4;i++)
       if(sharedMemory->Calls[i].callNum > max)
          {
             max=sharedMemory->Calls[i].callNum;
             sharedMemory->Tarnib=sharedMemory->Calls[i].TarnibType;
             sharedMemory->PlayerNum=(i+1);
          }
     sharedMemory->maxCall=max;
     printf("Highest Call %d , for Player %d , Tarnib Type Is %d \n",sharedMemory->maxCall,sharedMemory->PlayerNum,sharedMemory->Tarnib);
     release.sem_num = 0; 

     sharedMemory->Determine=sharedMemory->PlayerNum;   /*Player that Determine the Type
                                           * might be he highest call , or who win the round 
                                           */
    /*first Sempahore Decide which player will Start Playing 
     *after Decideing and Set the Tarnib Value , we Should give the Turns to Players to Start
     * Start With Player that Decide Tarnib
     */

    /*Give Sempahore for Highest Call*/
     sharedMemory->flag=1;    /*Always Reset the Flaf before Start Playing */
    arg.val=1;
    semctl(semid,sharedMemory->PlayerNum,SETVAL,arg);
    

  for(i=0;i<13;i++){
    printf("loop next\n");
   /*now all Players will play the First round , and Parent wait until the last Player done and Give him a Semaphore*/
    acquire.sem_num =0;   
 
    if ( semop(semid, &acquire, 1) == -1 ) {
      perror("semop -- producer -- acquire");
      exit(4);
     }
   /*print the value of cards that each player Played */
      for(i=0;i<4;i++)
        ;// printf("Player %d:Card %d,type %d\n",(i+1),sharedMemory->PlayedRoundCards[i].cardNum,sharedMemory->PlayedRoundCards[i].type);
 
    /*Decide which player won the round and give him the Semaphore to start Playing
     *Change the Player num and counter and flag
     */

    release.sem_num = 0;

    arg.val=1;
    semctl(semid,sharedMemory->PlayerNum,SETVAL,arg);
    sharedMemory->flag=1;
    
    }//*then loop*//

  
    //  for(i=0;i<4;i++);
    //  kill(pid_array[i],SIGKILL);

    //*Suspend the Main Process Now it will Listen to children until they finish there Work .*/
    //wait all processes  or KILL them 
  
}//Score reached to 61 , Exit the program

   for(i=0;i<4;i++);
     kill(pid_array[i],SIGKILL);
   for ( i = 0; i < 4; i++ ) {
      if (waitpid(pid_array[i], &status, 0) == pid_array[i]) {
	printf("Process ID %d has terminated\n", pid_array[i]);
      }
    }

    printf("All Processes has been terminated\n");
    printf("Parent removing shared memory\n");
    shmdt(sharedMemory);                             //*Disattached Shared Memory*//
    shmctl(shmid, IPC_RMID, (struct shmid_ds *) 0);  //*Delete Shared Memory *//
    printf("Parent removing Semaphore Set ...\n");
    semctl(semid, 0, IPC_RMID, 0);                   //*Delete Semaphore Setd	*//
    printf("Program is Exiting ...\n");

 return 0;
}	
