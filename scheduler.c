#include "headers.h"

bool stillSending = true;

void schedularHandler(int signum)
{
    stillSending = false;
}

int main(int argc, char * argv[])
{
    signal(SIGUSR1, schedularHandler);
    initClk();
    enum queueInsertionKey factor = atoi(argv[1]);
    switch (factor)
    {
    case HPF:
        printf("\nScheduled Algorithm is the (non) HPF\n");
        break;
    case SRTN:
        printf("\nScheduled Algorithm is the (SRTN)\n");
        break;
    case RR:
        printf("\nScheduled Algorithm is the (RR) and quantumNum:%s \n", argv[2]);
        break;
    case SJF:
        printf("\nScheduled Algorithm is the (SJF) \n");
        break;
    case FCFS:
        printf("\nScheduled Algorithm is the (FCFS) \n");
        break;
    }

    //Get Current Work Directory for Process.out
    char processBuffer[500];
    getcwd(processBuffer, sizeof(processBuffer));
    const char* PROCESS_PATH = strcat(processBuffer, "/process.out");
    
    //Creating Receiving Queue
    key_t keyID;
    keyID = ftok("keyfile", QUEUE_KEY);
    int receivingQueueID = msgget(keyID, 0666| IPC_CREAT);

    while(stillSending)
    {
        struct msgbuff received;
        int rec_val = msgrcv(receivingQueueID, &received, sizeof(struct process), 7, !IPC_NOWAIT);
        if(rec_val != -1)
        {
            printProcess(received.p);
        }
    }

    //Delete Queue
    msgctl(receivingQueueID, IPC_RMID, NULL);
    printf("\nFinshed Schedular\n");
    
    //TODO implement the scheduler :)
    //upon termination release the clock resources  
    
    destroyClk(false);
    exit(0);
}
