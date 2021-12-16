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
   struct node *next;
};

struct node* newNode(struct process* p)
{
   struct node* temp = (struct node*)malloc(sizeof(struct node));
   temp->p = p;
   temp->next = NULL;
   return temp;
}

struct customPriorityQueue
{
   struct node *head;
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
   return returnedProcess;
}

void enqueue(struct customPriorityQueue** q, struct process* p, enum queueInsertionKey i)
{
    if (isEmpty(q))
    {
        struct node *temp = newNode(p);
        ((*q)->head) = temp;
        return;
    }
    struct node *start = ((*q)->head);
    struct node *temp = newNode(p);

    switch(i)
    {
        case HPF:
            // enqueue acording to priority
            if (((*q)->head)->p->priority > p->priority)
            {
                temp->next = ((*q)->head);
                ((*q)->head) = temp;
            }
            else
            {
                // assuiming same priority will take arrival time
                while (start->next != NULL && start->next->p->priority <= p->priority)
                {
                    start = start->next;
                }
                temp->next = start->next;
                start->next = temp;
            }
            break;

        case SRTN:
            // enqueue acording to remainingTime
            if (((*q)->head)->p->remainingTime > p->remainingTime)
            {
                temp->next = ((*q)->head);
                ((*q)->head) = temp;
            }
            else
            {
                while (start->next != NULL && start->next->p->remainingTime <= p->remainingTime)
                {
                    start = start->next;
                }
                temp->next = start->next;
                start->next = temp;
            }
            break;

        case RR:
            // FIX: enqueue Last
            if (((*q)->head)->p->priority > p->priority)
            {
                temp->next = ((*q)->head);
                ((*q)->head) = temp;
            }
            else
            {
                while (start->next != NULL && start->next->p->priority <= p->priority)
                {
                    start = start->next;
                }
                temp->next = start->next;
                start->next = temp;
            }
            break;

        case SJF:
            // enqueue acording to runTime
            if (((*q)->head)->p->runTime > p->runTime)
            {
                temp->next = ((*q)->head);
                ((*q)->head) = temp;
            }
            else
            {
                while (start->next != NULL && start->next->p->runTime <= p->runTime)
                {
                    start = start->next;
                }
                temp->next = start->next;
                start->next = temp;
            }
            break;

        case FCFS:
            // enqueue (last) acording to arrivalTime
            if (((*q)->head)->p->priority > p->priority)
            {
                temp->next = ((*q)->head);
                ((*q)->head) = temp;
            }
            else
            {
                while (start->next != NULL && start->next->p->priority <= p->priority)
                {
                    start = start->next;
                }

                temp->next = start->next;
                start->next = temp;
            }
            break;
    }
}

void incrementWaintingTime(struct customPriorityQueue** q)
{
    if (isEmpty(q))
        return;
    struct node* temp = ((*q)->head);
    temp->p->waitingTime++;
    while (temp->next != NULL)
    {
        temp->next->p->waitingTime++;
        temp = temp->next;
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

void printProcess(struct process p)
{
    printf("\nCurrentTime:%d, ID = %d, ArrivalTime = %d, RunningTime = %d, Priority = %d.\n", getClk(), p.id, p.arrivalTime, p.runTime, p.priority);
}

void printQueue(struct customPriorityQueue** q)
{
    if (isEmpty(q))
        return;
    struct node* start = ((*q)->head);
    printProcess(*(start->p));
    while (start->next != NULL)
    {
        printProcess(*(start->next->p));
        start = start->next;
    }
}