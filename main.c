#include "header.c"

int main(int argc , char * argv[])
{
   /************************************ Creating Players ***********************************/
  int i , status;			
  pid_t pid, pid_array[4];    //*We have 4 Player Each Represented as Process , and Main Process Handle Them *//
  //*Temporary Variable that will hold the current Id of the Process , and an Array of Ids of Players Processes*//
  //*creating four Processes and Save there IDs in Parent Process array *//
  //* fork Return 0 in Child , And Children ID for the Parent , So that the Parent Process Knows his Children ID's*//
  
  char str[15];//*Send the Player Number to the Processs , USed to Synchronize Processes with Sempahores *//
  for ( i = 0; i < 4; i++ ) {
    pid = fork();
    
    if ( pid == 0 ) {
      printf("Player %d Ready\n",(i+1));      
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

  /***************************************** Allocating shared Memory **************************************/
  //*Create Shared Memory And Semaphores *//
  /*Create, attach and initialize the memory segment*/
  int shmid;        //*Shared Memory ID //
  pid=getpid();
  
  //*Allocate Shared Memory*//
  shmid = shmget((int) pid, sizeof(struct sharedMemory),IPC_CREAT | 0600);
  if(shmid==-1){
    perror("Shared Memory Failure");
    exit(0);
  }else {
    printf("Shared Memory created \n");
  }
  
  //*Creating Structre For shared Memory and Attached it to Parent Process*//
  Pointer sharedMemory = (Pointer)shmat(shmid, 0, 0);

  /********************************** Creating and Initialization Semaphore Set *************************************/
  //*Creating Semaphore Set*//
  //*Semaphore Set will Contain 5 Semaphores ,  one Semaphore For Each Player and One For Parent Process to Organize Work*//
 
  int semid;                 //*semaphore ID*//
  union semun    arg;
  static ushort  start_val[2]= {1,0};
  
  semid = semget((int) pid,5, IPC_CREAT | 0666);

  if(semid==-1){
     perror("Semaphore Fauilure");
     exit(0);
   }else {
            printf("Semaphore Set Created Sucessfully\n");
         }

   arg.array = start_val;
   //*initialize all Semaphore Set to 0 *//
    if ( semctl(semid,0, SETALL, arg) == -1 ) {
      perror("semctl -- parent -- initialization");
      exit(3);
    }

  /********************************** Start the Tarnib Game  *************************************/
  

  arg.val=1;
  semctl(semid,0,SETVAL,arg);   //*set the Value of Semaphore 0 to 1 , it can be Taken when calling Acquire*//
  //*Acquire Semaphore number 0 from Semaphore Set of the Val equal to 1 *//
  //* if val equal to zero Process Blocked *//
   
   /*Create and Initialize the Card Set and Save it to shared Memory*/
   List L = createList();
   initializeCards(L);
   sharedMemory->shuffledPlayer=1;   /*at begining the Player one Shuffled and Distribute the Cards */
   sharedMemory->TotalOneScore=0;
   sharedMemory->TotalTwoScore=0;
   sharedMemory->TeamOne=0;
   sharedMemory->TeamTwo=0;
   printf("GAME BEGIN\n");
   srand(time(NULL));

while(sharedMemory->TotalOneScore < 61 && sharedMemory->TotalTwoScore < 61){
    sleep(1);
   //*Get Semaphore Number 0 for the Parent Process *//
    acquire.sem_num =0; 
    if ( semop(semid, &acquire, 1) == -1 ) {
      perror("semop -- producer -- acquire");
      exit(4);
     }
    printf("Parent Process Acquire Semaphore\n");
    initializeCards(L);
    List shuffled=shuffle(L); 			             /*Shuffle the Card Set in Parent*/       
    StoreShuffuledToSharedMemory(sharedMemory,shuffled);     /*Store Cards into shared Memory*/ 
    printf("Player Number %d Shuffled and Distribuyed Cards \n",sharedMemory->shuffledPlayer);
    DistributeCards(sharedMemory,sharedMemory->shuffledPlayer);       /*Distribute Cards For Players , distributed By Player 1 */
                                                            /*NOTE : WE WILL CHANGE THE PLAYER WHO DISTRIBUTE THE CARDS */                
   /*THIS VALUE WILL CONTAIN THE PLAYER WHO SHUFFLED THE CARD SET , AND WILL INCREMENT EACH TIME */
   if(sharedMemory->shuffledPlayer!=4)
      sharedMemory->shuffledPlayer=sharedMemory->shuffledPlayer++;
   else
      sharedMemory->shuffledPlayer=1;

   /*when the Round End the Next Player will Shuffle and Distribute the Cards*/

   //*critical Part ends*//
   release.sem_num = 0;    //*Relase the Semaphore number 0 Critical Path ends *//

   /*Part One Finished , Decide Which Player To shuffle and Distribute the Cards Each Round , by Round Robin*/

   /********************************** Each Player Calculate his winning Rounds *************************************/

    //*Give Permission for other Process to Start Working individually*//   
    /*the Player Who start Calling is the Player next to the Shuffled Player */
    /*already the shuffled player Value increment to the next player*/

    arg.val=1;         
    semctl(semid,sharedMemory->shuffledPlayer,SETVAL,arg);   

    /*At this Point No One Calls any thing so We but a Flag to See if all Players Call to Return to Parent Process*/
    sharedMemory->flag=1;    /*Always Reset the Flag before Start Playing */   


    /*at the beginig first player should Shuffle the Cards 
     * then each player should see how many sub round the Could win and Update that in Shared Memory
     * then Parent should Moniter that every thing work ( last Player Give Semaphore to Parent to Moniter Work
     */ 

    //*Parent Freeze until Last Process Update the Number of Calls*//
    /* parent Wait Players to Store there Calls in Shared Memory to Decide Which Player will Start
      *last Player to Call must give Semaphore to Parent to continue Working */

  /********************************** Parent Process decide the Highest Call *************************************/  
    acquire.sem_num =0; 
 
    if ( semop(semid, &acquire, 1) == -1 ) {
      perror("semop -- producer -- acquire");
      exit(4);
     }
    
     /*Semaphore Acquired , Test if All players put there Calls 
      * parent will decide which Player will start by giving him a semaphore
      */ 

   /* test each player Calls */ 
   printf("all Players calls , Parent is Deciding which Player will start Playing \n");
      for(i=0;i<4;i++)
       { 
         printf("Player number %d  Calls %d ",(i+1),sharedMemory->Calls[i].callNum);
         printf("Tarnib type %d \n",sharedMemory->Calls[i].TarnibType);
       }
 
     /*now after all players Calls , Parent should Decide who will Decide the Tarnib Category and give him Sempahore To Start Playing
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
    //*Highest Call Determined *//
    //*Tarnib Type , Player Call , and Number of Calls will be Stored until the End of Each round to Evaluate Score *//
    
     /*Parent Will give Semaphore to Highest Call Player , player in Turn will be saved in Determine */

     sharedMemory->Determine=sharedMemory->PlayerNum; 
     sharedMemory->TeamNum=sharedMemory->PlayerNum;  

    /*first Sempahore Decide which player will Start Playing 
     *after Decideing and Set the Tarnib Value , we Should give the Turns to Players to Start
     * Start With Player that Decide Tarnib
     */

    /*Give Sempahore for Highest Call*/
    sharedMemory->flag=1;    /*Always Reset the Flag before Start Playing */
    arg.val=1;
    semctl(semid,sharedMemory->Determine,SETVAL,arg);
    int k=0;
   
   countRounds++;
   sleep(1);
   printf("Round # %d Begin \n",countRounds);
   //sleep(1);
   for(i=0;i<13;i++){
  
   /*now all Players will play the First round , and Parent wait until the last Player done and Give him a Semaphore*/
    acquire.sem_num =0;   

    if ( semop(semid, &acquire, 1) == -1 ) {
      perror("semop -- producer -- acquire");
      exit(4);
     }
   /*all players Decide which Card to Play and Played */
   /*Parent Process Should Decide the Winner of the Game */
   /*print the value of cards that each player Played */
     // for(k=0;k<4;k++)
       // printf("Player %d:Card %d,type %d\n",(k+1),sharedMemory->PlayedRoundCards[k].cardNum,sharedMemory->PlayedRoundCards[k].type);
     
    //*Team 1 is 0 + 2 , Team 2 is 1 + 3*//
     int win=0;
     win=DetermineWinner();
     if(win == 0 || win == 2)
       { 
          sharedMemory->TeamOne=sharedMemory->TeamOne++;
           if(win==0)
              sharedMemory->Determine=0;
           else 
	      sharedMemory->Determine=2;
       }else{
        sharedMemory->TeamTwo=sharedMemory->TeamTwo++;
           if(win==1)
              sharedMemory->Determine=1;
           else 
	      sharedMemory->Determine=3;
       }
    /*Decide which player won the round and give him the Semaphore to start Playing
     *Change the Player Determine and counter and flag
     */
    release.sem_num = 0;
     int t=0;
/*
     for(t=0;t<4;t++)
       {
	  sharedMemory->PlayedRoundCards[t].cardNum=-1;
          sharedMemory->PlayedRoundCards[t].type=-1;
       }
*/

    sharedMemory->flag=1;
    arg.val=1;
    semctl(semid,sharedMemory->Determine,SETVAL,arg);  
    
    }//*then loop , until round done*//
  if(sharedMemory->TeamNum==0 ||sharedMemory->TeamNum==2){
     if(sharedMemory->TeamOne >= sharedMemory->maxCall){
        sharedMemory->TotalOneScore=sharedMemory->TotalOneScore+sharedMemory->TeamOne;
        printf("Team 1 Won the Round , The Call %d , and They Get %d\n",sharedMemory->maxCall, sharedMemory->TeamOne); 
    }else{
        sharedMemory->TotalOneScore=sharedMemory->TotalOneScore-sharedMemory->TeamOne;
        printf("Team 1 Lost the Round , The Call %d , and They Get %d\n",sharedMemory->maxCall, sharedMemory->TeamOne);
        printf("Team 2 Get %d Points For Wining the round \n",(13-sharedMemory->TeamOne)); 
        sharedMemory->TotalTwoScore=sharedMemory->TotalTwoScore+(13-sharedMemory->TeamOne);
         }
  }else{
     if(sharedMemory->TeamTwo >= sharedMemory->maxCall){
        sharedMemory->TotalTwoScore=sharedMemory->TotalTwoScore+sharedMemory->TeamTwo;
        printf("Team 2 Won the Round , The Call %d , and They Get %d\n",sharedMemory->maxCall, sharedMemory->TeamTwo); 
    }else{
        sharedMemory->TotalTwoScore=sharedMemory->TotalTwoScore-sharedMemory->TeamTwo;
        printf("Team 2 Lost the Round , The Call %d , and They Get %d\n",sharedMemory->maxCall, sharedMemory->TeamTwo);
        printf("Team 1 Get %d Points For Wining the round \n",(13-sharedMemory->TeamTwo)); 
        sharedMemory->TotalOneScore=sharedMemory->TotalOneScore+(13-sharedMemory->TeamTwo);
         }
       }
       printf("Total Score\n"); 
       printf("Team One  Score equal %d\n",sharedMemory->TotalOneScore);
       printf("Team Two  Score equal %d\n",sharedMemory->TotalTwoScore);

  sharedMemory->TeamTwo=0;
  sharedMemory->TeamOne=0;

}//Score reached to 61 , Exit the program
  sleep(1);
  if(sharedMemory->TotalTwoScore <sharedMemory->TotalOneScore)
    {
       printf("Team One Win Score equal %d\n",sharedMemory->TotalOneScore);
       printf("Team Two  Score equal %d\n",sharedMemory->TotalTwoScore);
  
     }else {
               printf("Team Two win Score equal %d\n",sharedMemory->TotalTwoScore);
	       printf("Team One Score equal %d\n",sharedMemory->TotalOneScore);
           }
  for(i=0;i<4;i++)
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
