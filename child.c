#include "header.c"

int main(int argc, char*argv[])
{
  //sleep(120);
  /********************************** Initialize Shared Memory And Semaphore Set *************************************/		
  int shmid;
  int pid=getppid();
  shmid = shmget((int) pid, 0,0);                                     /*Sahred Memory key is Equal to Parent ID*/
  if(shmid==-1){
     perror("Shared Memory");
     exit(0);
  }else{
         printf("Shared Memory Attached to Player %d \n", atoi(argv[1]));
       }

  Pointer sharedMemory = (Pointer)shmat(shmid, 0, 0);                 /*attach the Player to Shared Memory*/   
	
  int semid;
  union semun arg;

  if ( (semid = semget((int) getppid(), 5, 0)) == -1 ) {
    perror("semget -- producer -- access");
    exit(3);
   }

  srand(time(NULL));
  /*sequence will continue Until one Team Reach to 61*/

while(sharedMemory->TotalOneScore < 61 && sharedMemory->TotalTwoScore < 61){
   
    /********************************** Calculate the number of Winning Sub round *************************************/
    /*Try to acquire Sempahore for Process , Set the Semaphore number to player number */
    /*first semaphore will make player Call a number of sub rounds and Tarnib */
    acquire.sem_num = atoi(argv[1]);
    
    if ( semop(semid, &acquire, 1) == -1 ) {
      perror("semop -- producer -- acquire");
      exit(4);
    }
    printf(" Player Number is %s  is counting  his winning sub Round \n",argv[1]);
   
    /*Decide how Many Winning Sub round and Update it into shared Memory */
    /*Useless if Statement*/

     if(atoi(argv[1])==atoi(argv[1]))
         {
          int Tar;
          int x= getNumOfSubRounds(sharedMemory ,atoi(argv[1]),&Tar);
          printf("Number of Possible Winning Hand %d , Tarnib Type is %d \n",x,Tar);
          int i;
          if(x>6)
           {
             sharedMemory->Calls[atoi(argv[1])-1].callNum=x;
             sharedMemory->Calls[atoi(argv[1])-1].TarnibType=Tar;
           }else{
                 sharedMemory->Calls[atoi(argv[1])-1].callNum=-1;
                 sharedMemory->Calls[atoi(argv[1])-1].TarnibType=0;
                }
          /* test if the cards shuffled and Distributed for Players
           for(i=0;i<13;i++)
           {

             printf(" %d ",sharedMemory->player1[i].type);
             printf(" %d\n",sharedMemory->player1[i].cardNum);
           }*/
               
         }
       //  printSharedSet(sharedMemory);

    release.sem_num = atoi(argv[1]);                         /*relase the Semaphore , critical Part ends */
     

    arg.val=1;                                     /* give a semaphore to the next Process*/
    //flag = 4 that means all four players Turn Finish , 
    if(sharedMemory->flag!=4)
      {
        sharedMemory->flag=sharedMemory->flag++;
        //if we are in Player four and turns Does Not end , 
        //Go to Player 1 and Continue Playing
        if(atoi(argv[1])== 4)
        {
          semctl(semid,1,SETVAL,arg);
        }else{
               //else we give the semaphore to the next Player to Play
               semctl(semid,atoi(argv[1])+1,SETVAL,arg);
             }
      }else { 
               //if flag =4 then all turns finish we should Return to Parent To Compute Results
               sharedMemory->flag=0;  //*indicate that we are giving semaphore for Parent*//
               // give sempahore to Parent Process , and Reset flag for Players to Start again after computing Results
               semctl(semid,0,SETVAL,arg);
            }

  /********************************** Start Playing SubRounds *************************************/

    /*After Finish the Loop for Deciding Highest Call , Parent Process Give Semaphore 
     * to Specific Player to Play a Card 
     */   

   //*at the begining each player will have 13 cards , so we will finish the rounds after each player played 13 times *//
   int i;
   for(i=0;i<13;i++){ 
    //*Player Who Acquire the Semaphore Has the Highest Call first*//
    //*Now all Players Are Waiting For Parent to Decide the highest Call to start Playing*//
    acquire.sem_num = atoi(argv[1]);

    if ( semop(semid, &acquire, 1) == -1 ) {
      perror("semop -- producer -- acquire");
      exit(4);
    }

    chooseCard(sharedMemory,atoi(argv[1]));
   /* printf("Card is %d type :%d\n",sharedMemory->PlayedRoundCards[atoi(argv[1])].cardNum,sharedMemory->PlayedRoundCards[atoi(argv[1])].type);*/
    //*in this part we will decide which Card to Play*// 
    release.sem_num = atoi(argv[1]);

    arg.val=1;                                     /* give a semaphore to the next Process*/
    //flag = 4 that means all four players Turn Finish , 
    if(sharedMemory->flag!=4)
      {
        sharedMemory->flag=sharedMemory->flag++;
        //if we are in Player four and turns Does Not end , 
        //Go to Player 1 and Continue Playing
        if(atoi(argv[1])== 4)
        {
          semctl(semid,1,SETVAL,arg);
        }else{
               //else we give the semaphore to the next Player to Play
               semctl(semid,atoi(argv[1])+1,SETVAL,arg);
             }
      }else { 
               //if flag =4 then all turns finish we should Return to Parent To Compute Results
               sharedMemory->flag=0;
               // give sempahore to Parent Process , and Reset flag for Players to Start again after computing Results
               semctl(semid,0,SETVAL,arg);
            }

  }//end of while loop , end of round 
 }/*Game Finished When One of the Teams Reach to 61*/
  exit(0);
  return 0;
}
void chooseCard(Pointer sharedMemory, int PlayerNum)
{
  if(sharedMemory==NULL)
   {
     perror("Shared Memory");
     exit(0);
   }else {
            if(sharedMemory->Determine==PlayerNum)
               {
                  /*choose the Highest unused Card between your cards*/
                  int i;
 		  int max=0,type=0,index=-1;
                  switch(PlayerNum)
                   {
			case 1: for(i=0;i<13;i++)
 				{
				  if(sharedMemory->player1[i].cardNum>max && sharedMemory->player1[i].state ==0)
                                     {
 					max=sharedMemory->player1[i].cardNum;
				  	type=sharedMemory->player1[i].type;
					index=i;
                                     }
				}
				sharedMemory->player1[index].state=1;
				sharedMemory->Category=type;
				sharedMemory->PlayedRoundCards[0].cardNum=max;
				sharedMemory->PlayedRoundCards[0].type=type;
				break;

			case 2: for(i=0;i<13;i++)
 				{
				  if(sharedMemory->player2[i].cardNum>max && sharedMemory->player2[i].state ==0)
                                     {
 					max=sharedMemory->player2[i].cardNum;
				  	type=sharedMemory->player2[i].type;
					index=i;
                                     }
				}
				sharedMemory->player2[index].state=1;
				sharedMemory->Category=type;
				sharedMemory->PlayedRoundCards[1].cardNum=max;
				sharedMemory->PlayedRoundCards[1].type=type;
				break;

			case 3: for(i=0;i<13;i++)
 				{
				  if(sharedMemory->player3[i].cardNum>max && sharedMemory->player3[i].state ==0)
                                     {
 					max=sharedMemory->player3[i].cardNum;
				  	type=sharedMemory->player3[i].type;
					index=i;
                                     }
				}
				sharedMemory->player3[index].state=1;
				sharedMemory->Category=type;
				sharedMemory->PlayedRoundCards[2].cardNum=max;
				sharedMemory->PlayedRoundCards[2].type=type;
				break;

			case 4: for(i=0;i<13;i++)
 				{
				  if(sharedMemory->player4[i].cardNum>max && sharedMemory->player4[i].state ==0)
                                     {
 					max=sharedMemory->player4[i].cardNum;
				  	type=sharedMemory->player4[i].type;
					index=i;
                                     }
				}
				sharedMemory->player4[index].state=1;
				sharedMemory->Category=type;
				sharedMemory->PlayedRoundCards[3].cardNum=max;
				sharedMemory->PlayedRoundCards[3].type=type;
				break;

                   }
               }else {
                     /* if he is not the first player to play */
                    /* we should find the largest card from a specific Category of Play */
                     int  max=0, i , type=0, index=-1, flag=0;
                  switch(PlayerNum)
                   {
			case 1: for(i=0;i<13;i++)
 				{
				  if(sharedMemory->player1[i].cardNum>max && sharedMemory->player1[i].state ==0 )
                                     { 
                                         if(sharedMemory->player1[i].type=sharedMemory->Category){
 				            max=sharedMemory->player1[i].cardNum;
				  	    type=sharedMemory->player1[i].type;
					    index=i;
					     flag=1;
                                           }
                                     }
				}
                               //if flag equal to one means that we have a card from the same  category //
                                if(flag==1){
				  sharedMemory->player1[index].state=1;
				  sharedMemory->PlayedRoundCards[0].cardNum=max;
				  sharedMemory->PlayedRoundCards[0].type=type;
                                }else {
   					int fl=0;
					//*we must choose first Tarnib Card We Face , to make it simple*//
					for(i=0;i<13;i++)
					  {
						if(sharedMemory->player1[i].type==sharedMemory->Tarnib)
                                                  {
							sharedMemory->PlayedRoundCards[0].cardNum=sharedMemory->player1[i].cardNum;
							sharedMemory->PlayedRoundCards[0].type=sharedMemory->player1[i].type;
							sharedMemory->player1[i].state=1;
							fl=1;
							break;
						  }
					  }
                                        if(fl==0)
					{
						/*we don't Have any Tarnib Cards , just throw Any Card */
                                                for(i=0;i<13;i++)
                                                  {
							if(sharedMemory->player1[i].state==0){
							sharedMemory->PlayedRoundCards[0].cardNum=sharedMemory->player1[i].cardNum;
							sharedMemory->PlayedRoundCards[0].type=sharedMemory->player1[i].type;
							sharedMemory->player1[i].state=1;
							break;
							}
 						  }
					}
				      }
				break;

			case 2:  for(i=0;i<13;i++)
 				{
				  if(sharedMemory->player2[i].cardNum>max && sharedMemory->player2[i].state ==0 )
                                     { 
                                         if(sharedMemory->player2[i].type=sharedMemory->Category){
 				            max=sharedMemory->player2[i].cardNum;
				  	    type=sharedMemory->player2[i].type;
					    index=i;
					     flag=1;
                                           }
                                     }
				}
                               //if flag equal to one means that we have a card from the same  category //
                                if(flag==1){
				  sharedMemory->player2[index].state=1;
				  sharedMemory->PlayedRoundCards[1].cardNum=max;
				  sharedMemory->PlayedRoundCards[1].type=type;
                                }else {
   					int fl=0;
					//*we must choose first Tarnib Card We Face , to make it simple*//
					for(i=0;i<13;i++)
					  {
						if(sharedMemory->player2[i].type==sharedMemory->Tarnib)
                                                  {
							sharedMemory->PlayedRoundCards[1].cardNum=sharedMemory->player2[i].cardNum;
							sharedMemory->PlayedRoundCards[1].type=sharedMemory->player2[i].type;
							sharedMemory->player2[i].state=1;
							fl=1;
							break;
						  }
					  }
                                        if(fl==0)
					{
						/*we don't Have any Tarnib Cards , just throw Any Card */
                                                for(i=0;i<13;i++)
                                                  {
							if(sharedMemory->player2[i].state==0){
							sharedMemory->PlayedRoundCards[1].cardNum=sharedMemory->player2[i].cardNum;
							sharedMemory->PlayedRoundCards[1].type=sharedMemory->player2[i].type;
							sharedMemory->player2[i].state=1;
							break;
							}
 						  }
					}
				      }
				break;

			case 3:  for(i=0;i<13;i++)
 				{
				  if(sharedMemory->player3[i].cardNum>max && sharedMemory->player3[i].state ==0 )
                                     { 
                                         if(sharedMemory->player3[i].type=sharedMemory->Category){
 				            max=sharedMemory->player3[i].cardNum;
				  	    type=sharedMemory->player3[i].type;
					    index=i;
					     flag=1;
                                           }
                                     }
				}
                               //if flag equal to one means that we have a card from the same  category //
                                if(flag==1){
				  sharedMemory->player3[index].state=1;
				  sharedMemory->PlayedRoundCards[2].cardNum=max;
				  sharedMemory->PlayedRoundCards[2].type=type;
                                }else {
   					int fl=0;
					//*we must choose first Tarnib Card We Face , to make it simple*//
					for(i=0;i<13;i++)
					  {
						if(sharedMemory->player3[i].type==sharedMemory->Tarnib)
                                                  {
							sharedMemory->PlayedRoundCards[2].cardNum=sharedMemory->player3[i].cardNum;
							sharedMemory->PlayedRoundCards[2].type=sharedMemory->player3[i].type;
							sharedMemory->player3[i].state=1;
							fl=1;
							break;
						  }
					  }
                                        if(fl==0)
					{
						/*we don't Have any Tarnib Cards , just throw Any Card */
                                                for(i=0;i<13;i++)
                                                  {
							if(sharedMemory->player3[i].state==0){
							sharedMemory->PlayedRoundCards[2].cardNum=sharedMemory->player3[i].cardNum;
							sharedMemory->PlayedRoundCards[2].type=sharedMemory->player3[i].type;
							sharedMemory->player3[i].state=1;
							break;
							}
 						  }
					}
				      }
				break;
				
			case 4:  for(i=0;i<13;i++)
 				{
				  if(sharedMemory->player4[i].cardNum>max && sharedMemory->player4[i].state ==0 )
                                     { 
                                         if(sharedMemory->player4[i].type=sharedMemory->Category){
 				            max=sharedMemory->player4[i].cardNum;
				  	    type=sharedMemory->player4[i].type;
					    index=i;
					     flag=1;
                                           }
                                     }
				}
                               //if flag equal to one means that we have a card from the same  category //
                                if(flag==1){
				  sharedMemory->player4[index].state=1;
				  sharedMemory->PlayedRoundCards[3].cardNum=max;
				  sharedMemory->PlayedRoundCards[3].type=type;
                                }else {
   					int fl=0;
					//*we must choose first Tarnib Card We Face , to make it simple*//
					for(i=0;i<13;i++)
					  {
						if(sharedMemory->player4[i].type==sharedMemory->Tarnib)
                                                  {
							sharedMemory->PlayedRoundCards[3].cardNum=sharedMemory->player4[i].cardNum;
							sharedMemory->PlayedRoundCards[3].type=sharedMemory->player4[i].type;
							sharedMemory->player4[i].state=1;
							fl=1;
							break;
						  }
					  }
                                        if(fl==0)
					{
						/*we don't Have any Tarnib Cards , just throw Any Card */
                                                for(i=0;i<13;i++)
                                                  {
							if(sharedMemory->player4[i].state==0){
							sharedMemory->PlayedRoundCards[3].cardNum=sharedMemory->player4[i].cardNum;
							sharedMemory->PlayedRoundCards[3].type=sharedMemory->player4[i].type;
							sharedMemory->player4[i].state=1;
							break;
							}
 						  }
					}
				      }
				break;

                   }
                     }
         }
}

void Selectfirst(Pointer sharedMemory , int PlayerNum)
{
   //*we wil select Cards Directly from an Array , and place 1 to state when Select a card*//
   if(sharedMemory==NULL)
   {
      perror("Shared Memory");
      exit(0);
   }else{
	  srand(time(NULL));
       
          int index=rand()%13;
          while(sharedMemory->player1[index].state ==1)
               index=rand()%13;
          
	  sharedMemory->player1[index].state=1;
          sharedMemory->PlayedRoundCards[PlayerNum-1].cardNum=sharedMemory->player1[index].cardNum;
          sharedMemory->PlayedRoundCards[PlayerNum-1].type=sharedMemory->player1[index].type;

        }

}
void SelectCard(List L , Pointer sharedMemory, int PlayerNum)
{
   /*Selecting a card Depending On two Thing
    * if he is the first Player to Play , he will choose the Highest Non Tarnib Card
    * else he will see other Player played Cards in Shared Memory 
    * if he have a larger Card than Opposite he use it
    * if he has not , he use the smallest Tarnib Card he have 
    * if he has not any of the above option he use the smallest Card of any type
    * if some one Played a Tarnib Card in Opening , All players should play Tarnib
    */
   if(L==NULL)
    {
      perror("List does Not Exist");
      exit(0);
    }else if( sharedMemory==NULL)
          {
  	     perror("Shared Memory");
   	     exit(0);
          }else {
                   if(isEmpty(L))
                     {
 			perror("Player Have no Cards");
 			exit(0);
                     }

  		  /*if this is the first shoot*/
                  /*choose the largest Card */
 		  if(PlayerNum == sharedMemory->Determine)
                    {
                       int i,max=0,category=0;
                       Position P=L->next;
		       Position temp=L;
                       Position maxNode=NULL;
                       while(P!=NULL)
                       {
			 if(P->cardNum> max)
 			    {
				max=P->cardNum;
 				category=P->type;
                                if(maxNode==NULL){
  				   maxNode=P;                      /*remove the maximum Card From the Cards*/
                                   L->next=P->next;
  				}else{
                                        Position temp2=maxNode;
                                        maxNode=P;
					L->next=P->next;
					insert(L,temp2);
				     }
		 	    }
			 P=P->next;
                         maxNode->next=NULL;
 			 temp=temp->next;
                       }
			free(maxNode);
			sharedMemory->PlayedRoundCards[PlayerNum-1].cardNum=max;
                        sharedMemory->PlayedRoundCards[PlayerNum-1].type=category;
    		        sharedMemory->Category=category;
			sharedMemory->Determine=-1;
                    }else{
 			  /*this is not the first Player*/
 			  /* we should play a Card from the Same Category*/
                          Position P , temp;
                          int cat=sharedMemory->Category;
                          P=L->next;
	       		  temp=L;
			  int card,cats;
                          while(P!=NULL)
 			    {
			       if(P->type==sharedMemory->Category)
                                  {
					/*remove it from the set*/
					temp->next=P->next;
					card=P->cardNum;
					cats=P->type;
					sharedMemory->PlayedRoundCards[PlayerNum-1].cardNum=card;
                                        sharedMemory->PlayedRoundCards[PlayerNum-1].type=cats;
					break;
                                  }
 				P=P->next;
				temp=temp->next;
			    }
                         }
               }
}
int isUsed(Pointer sharedMemory, int PlayerNum)
{
     int i;
     int flag=0;
     switch(PlayerNum)
        {
		case 1:for(i=0;i<13;i++)
                           if(sharedMemory->player1[i].state==0){
                              flag=1;
 		   	      break;
			   }
                       break;
		case 2:for(i=0;i<13;i++)
                           if(sharedMemory->player2[i].state==0){
                              flag=1;
 		   	      break;
			   }
                       break;
		case 3:for(i=0;i<13;i++)
                           if(sharedMemory->player3[i].state==0){
                              flag=1;
 		   	      break;
			   }
                       break;
		case 4:for(i=0;i<13;i++)
                           if(sharedMemory->player4[i].state==0){
                              flag=1;
 		   	      break;
			   }
                       break;   
       }
   if(flag==1)
    {
  	return 0;
    }else {
		return 1;
          }

}
void toLinkedList(List L , int playerNum,Pointer sharedMemory)
{
  if(L==NULL)
   {
      perror("List does not exist");
      exit(0);
   }else{
           int i=0;
           Position P;
           switch(playerNum)
            {
		case 1:while(i<13)
                       {
         		   P=(Position)malloc(sizeof(struct node));
                           P->cardNum=sharedMemory->player1[i].cardNum;
  			   P->type=sharedMemory->player1[i].type;
                           i++;
                           insert(L,P);
                       }
                       break;
		case 2:while(i<13)
                       {
         		   P=(Position)malloc(sizeof(struct node));
                           P->cardNum=sharedMemory->player2[i].cardNum;
  			   P->type=sharedMemory->player2[i].type;
                           i++;
                           insert(L,P);
                       }
                       break;
		case 3:while(i<13)
                       {
         		   P=(Position)malloc(sizeof(struct node));
                           P->cardNum=sharedMemory->player3[i].cardNum;
  			   P->type=sharedMemory->player3[i].type;
                           i++;
                           insert(L,P);
                       }
                       break;
		case 4:while(i<13)
                       {
         		   P=(Position)malloc(sizeof(struct node));
                           P->cardNum=sharedMemory->player4[i].cardNum;
  			   P->type=sharedMemory->player4[i].type;
                           i++;
                           insert(L,P);
                       }
                       break;   
            }
        }
}

