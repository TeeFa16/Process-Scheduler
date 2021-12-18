#include <stdio.h>      //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <time.h>


typedef short bool;
#define true 1
#define false 0

#define SHKEY 300

#define QUEUE_KEY 90


///==============================
//don't mess with this vaSSSriable//
int * shmaddr;                 //
//===============================



int getClk()
{
    return *shmaddr;
}


/*
 * All process call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *) shmat(shmid, (void *)0, 0);
}


/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
*/

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}

// structs
struct processInputLine 
{
    int processID;
    int processArrTime;
    int processRunTime;
    int processPriority;
};

struct process 
{
    int id;
    int pID;
    char state[20];
    int arrivalTime;
    int priority;
    int runTime;
    int remainingTime;
    int waitingTime;
    int finishedTime;
    int turnAround;
    float weightedTurnAround;
};

struct msgbuff
{
    long mtype;
    struct process p;
};

enum queueInsertionKey
{
    HPF = 1,
    SRTN = 2,
    RR = 3,
    SJF = 4,
    FCFS = 5,
};

//Priority Queue
struct node
{
   struct process* p;
   struct node* next;
};

struct node* newNode(struct process pObj)
{
    struct node* temp = (struct node*)malloc(sizeof(struct node));
    
    struct process* newProcess = (struct process*)malloc(sizeof(struct process));
    newProcess->id = pObj.id;
    newProcess->pID = pObj.pID;
    strcpy(newProcess->state, pObj.state);
    newProcess->arrivalTime = pObj.arrivalTime;
    newProcess->runTime = pObj.runTime;
    newProcess->priority = pObj.priority;
    newProcess->remainingTime = pObj.remainingTime;
    newProcess->waitingTime = pObj.waitingTime;
    newProcess->weightedTurnAround = pObj.weightedTurnAround;
    newProcess->turnAround = pObj.turnAround;
    temp->p = newProcess;
    temp->next = NULL;
    return temp;
}

struct customPriorityQueue
{
   struct node* head;
   int count;
};

struct customPriorityQueue* newCustomPriorityQueue()
{
   struct customPriorityQueue* temp = (struct customPriorityQueue*)malloc((sizeof(struct customPriorityQueue)));
   (temp->head) = NULL;
   temp->count = 0;
   return temp;
}

int isEmpty(struct customPriorityQueue** q)
{
   return (*q)->head == NULL;
}

struct process* front(struct customPriorityQueue** q)
{
   if (isEmpty(q))
   {
      struct process* p=NULL;
      return p;
   }
   return ((*q)->head->p);
}

struct process* dequeue(struct customPriorityQueue** q)
{
    if (isEmpty(q))
    {
        struct process* p=NULL;
        return p;
    }
    struct node *temp = ((*q)->head);
    ((*q)->head) = ((*q)->head)->next;
    struct process* returnedProcess = temp->p; 
    free(temp);
    (*q)->count--;
    return returnedProcess;
}

void printProcess(struct process* p)
{
   printf("id is: %d\n", p->id);
   printf("pID is: %d\n", p->pID);
   printf("arrival times is: %d\n", p->arrivalTime);
   printf("runTime is: %d\n", p->runTime);
   printf("prioriry is: %d\n", p->priority);
   printf("state is: %s\n", p->state);
   printf("remaining time is: %d\n", p->remainingTime);
   printf("finish time is: %d\n", p->finishedTime);
   printf("Waiting time is: %d\n", p->waitingTime);
   printf("TA is: %d\n", p->turnAround);
   printf("WTA is: %f\n", p->weightedTurnAround);
}

void enqueue(struct customPriorityQueue **q, struct process pObj, enum queueInsertionKey i)
{
    if(isEmpty(q))
    {
        struct node* temp = newNode(pObj);
        ((*q)->head) = temp;
        (*q)->count++;
        return;
    }
    struct node* start = ((*q)->head);
    struct node* temp = newNode(pObj);
    switch(i)
    {
        case HPF:
            if (((*q)->head)->p->priority > pObj.priority)
            {
                temp->next = ((*q)->head);
                ((*q)->head) = temp;
            }
            else
            {
                while (start->next != NULL && start->next->p->priority <= pObj.priority)
                {
                    start = start->next;
                }
                temp->next = start->next;
                start->next = temp;
            }
            (*q)->count++;
            break;
        case SRTN:
            if (((*q)->head)->p->remainingTime > pObj.remainingTime)
            {
                temp->next = ((*q)->head);
                ((*q)->head) = temp;
            }
            else
            {
                while (start->next != NULL && start->next->p->remainingTime <= pObj.remainingTime)
                {
                    start = start->next;
                }
                temp->next = start->next;
                start->next = temp;
            }
            (*q)->count++;
            break;
        case RR:
            ; 
            // RR append last   
            const int APPEND_LAST = 0;
            if (APPEND_LAST > APPEND_LAST)
            {
                temp->next = ((*q)->head);
                ((*q)->head) = temp;
            }
            else
            {
                while (start->next != NULL && APPEND_LAST <= APPEND_LAST)
                {
                    start = start->next;
                }
                temp->next = start->next;
                start->next = temp;
            }
            (*q)->count++;
            break;
        case SJF:
            if (((*q)->head)->p->runTime > pObj.runTime)
            {
                temp->next = ((*q)->head);
                ((*q)->head) = temp;
            }
            else
            {
                while (start->next != NULL && start->next->p->runTime <= pObj.runTime)
                {
                    start = start->next;
                }
                temp->next = start->next;
                start->next = temp;
            }
            (*q)->count++;
            break;
        case FCFS:
            if (((*q)->head)->p->arrivalTime > pObj.arrivalTime)
            {
                temp->next = ((*q)->head);
                ((*q)->head) = temp;
            }
            else
            {
                while (start->next != NULL && start->next->p->arrivalTime <= pObj.arrivalTime)
                {
                    start = start->next;
                }
                temp->next = start->next;
                start->next = temp;
            }
            (*q)->count++;
            break;
    }
}

void incrementWaintingTime(struct customPriorityQueue** q)
{
    if (isEmpty(q))
        return;
    struct node* tempPTR = ((*q)->head);
    tempPTR->p->waitingTime++;
    while (tempPTR->next != NULL)
    {
        tempPTR->next->p->waitingTime++;
        tempPTR = tempPTR->next;
    }
}

// utilities
void printList(struct process inputProccesses[], int inputProccessesCount)
{
    for(int i=0; i<inputProccessesCount; i++)
    {
        printf("\nProccess '%d, %d, %d, %d'\n", inputProccesses[i].id, inputProccesses[i].arrivalTime, inputProccesses[i].runTime, inputProccesses[i].priority);
    }
}


void printQueue(struct customPriorityQueue **q)
{
   if (isEmpty(q))
    {
        printf("\nEmpty Queue\n");
        return;
    }
   printf("\nStart of the Q, Count: %d\n", (*q)->count);
   struct node* start = ((*q)->head);
   printProcess(start->p);
   while (start->next != NULL)
   {
      printProcess(start->next->p);
      start = start->next;
   }
   printf("\nEND of the Q\n");
}