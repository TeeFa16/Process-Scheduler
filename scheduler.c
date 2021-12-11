#include "headers.h"

bool stillSending = true;


void schedularHandler(int signum) // THe SIGUSER1 signal handler
{
    stillSending = false;
}

int main(int argc, char * argv[])
{
    signal(SIGUSR1, schedularHandler);
    initClk();

    printf("\nScheduled Algorithm recieved (%s)\n", argv[1]);
    printf("\nScheduled Algorithm recieved Q (%s)\n", argv[2]);



    // switch (argv[1])
    // {
    // case 1:
    //     printf("\n Scheduled Algorithm is the (non) HPF\n");
    //     break;
    // case 2:
    //     printf("\nScheduled Algorithm is the (SRTN)\n");
    //     break;
    // case 3:
    //     printf("\nScheduled Algorithm is the (RR) and quantumNum:%s \n", argv[2]);
    //     break;
    // case 4:
    //     printf("\nScheduled Algorithm is the (SJF) \n");
    //     break;
    // case 5:
    //     printf("\nScheduled Algorithm is the (FCFS) \n");
    //     break;
    // }
    
    //Creating Receiving Queue
    key_t keyID;
    keyID = ftok("keyfile", 90);
    int receivingQueueID = msgget(keyID, 0666| IPC_CREAT);

    while(stillSending)
    {
        struct msgbuff received;
        int rec_val = msgrcv(receivingQueueID, &received, sizeof(struct process), 7, IPC_NOWAIT);

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
