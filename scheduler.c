#include "headers.h"

bool stillSending = true;


void schedularHandler(int signum) // THe SIGUSER1 signal handler
{
    printf("\nRecieved signal: %d \n", stillSending);
    stillSending = false;
    printf("\n Recived signal after: %d\n", stillSending);
}

int main(int argc, char * argv[])
{
    signal(SIGUSR1, schedularHandler);
    printf("\n Hello scheduler ID: %d\n", getpid());
    initClk();

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
    

    printf("\nteeeefa gameeeeed\n");
    while(stillSending)
    {
        // printf("\nteeeefa 3arsssssssss\n");
        // printf("\n %d\n", stillSending);
    }
    printf("\nFinshed Schedular\n");

    
    //TODO implement the scheduler :)
    //upon termination release the clock resources  
    
    destroyClk(false);
    exit(0);
}
