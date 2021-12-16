#include "headers.h"

struct customPriorityQueue* CPQptr = NULL;
struct process* runningProcessPTR = NULL;
bool stillSending = true;
const char* PROCESS_PATH = NULL;

void printDEBUG(struct process p)
{
    printf("\nAt\ttime\t%d\tprocess\t%d\t%s\tarr\t%d\tpriority\t%d\tremain\t%d\n", getClk(), p.id, p.state, p.arrivalTime, p.priority, p.remainingTime);
}

int wakeProccess(int runTime)
{
    // wake up a process
    char buffer[100];
    sprintf(buffer, "%d", runTime);
    const int PROCESS_PID = fork();
    if (PROCESS_PID == -1)
    {
        perror("error while forking a process");
    }
    else if (PROCESS_PID == 0)
    {
        execl(PROCESS_PATH, "process.out", buffer, NULL);
    }
    return PROCESS_PID;
}

void schedularHandler(int signum)
{
    stillSending = false;
}

void sigChildHandler(int signum)
{
    strcpy(runningProcessPTR->state, "finished");
    // TODO: store for analytics
    // Print to LOG file
    printDEBUG(*runningProcessPTR);
    runningProcessPTR = dequeue(&CPQptr);
    if(!strcmp(runningProcessPTR->state, "ready"))
    {
        strcpy(runningProcessPTR->state, "started");
        runningProcessPTR->pID = wakeProccess(runningProcessPTR->runTime);
    }
    else
    {
        strcpy(runningProcessPTR->state, "resumed");
        kill(runningProcessPTR->pID, SIGCONT);
    }
    // TODO:
    // Print to LOG file 
    printDEBUG(*runningProcessPTR);

}

int main(int argc, char * argv[])
{
    signal(SIGUSR1, schedularHandler);
    signal(SIGCHLD, sigChildHandler);
    initClk();
    enum queueInsertionKey insertionFactor = atoi(argv[1]);
    // DEBUG: Remove me
    switch (insertionFactor)
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
    // Get Current Work Directory for Process.out
    char processBuffer[500];
    getcwd(processBuffer, sizeof(processBuffer));
    const char* PROCESS_PATH = strcat(processBuffer, "/process.out");
    // Retriveing receivingQueueID
    key_t keyID;
    keyID = ftok("keyfile", QUEUE_KEY);
    int receivingQueueID = msgget(keyID, 0666| IPC_CREAT);
    // The Holy While "Schedular Implementation"
    CPQptr = newCustomPriorityQueue();
    while(stillSending || runningProcessPTR!=NULL || !isEmpty(&CPQptr))
    {
        struct msgbuff receivedProccessMSG;
        int rec_val = msgrcv(receivingQueueID, &receivedProccessMSG, sizeof(struct process), 7, !IPC_NOWAIT);
        if(rec_val != -1)
        {
            // intialize process struct
            struct process receivedProcess = receivedProccessMSG.p;
            strcpy(receivedProcess.state, "ready");
            receivedProcess.waitingTime = 0;
            receivedProcess.remainingTime = receivedProcess.runTime;
            /*
                > Now we need to know shall we run, or enqueue receivedProcess
                ready:    NO proccess.out
                started:  proccess.out Running
                stopped:  proccess.out EXISTS
                resumed:  proccess.out Running
                finished: proccess.out exited
            */
            // RECIEVED::CASE 1
            if(runningProcessPTR==NULL)
            {
                // TODO: run receivedProcess
                strcpy(runningProcessPTR->state, "started");
                runningProcessPTR->pID = wakeProccess(runningProcessPTR->runTime);
                // TODO:
                // Print to LOG file 
                printDEBUG(*runningProcessPTR);
            }
            // RECIEVED::CASE 2
            if(runningProcessPTR!=NULL)
            {
                // SRTN Case
                if(insertionFactor==SRTN && runningProcessPTR->remainingTime>receivedProcess.remainingTime)
                {
                    // TODO: replace runningProcess with receivedProcess
                    // [1]: stop runningProcess
                    strcpy(runningProcessPTR->state, "stopped");
                    kill(runningProcessPTR->pID, SIGSTOP);
                    enqueue(&CPQptr, runningProcessPTR, insertionFactor);
                    // [2]: Print to LogFile
                    printDEBUG(*runningProcessPTR);
                    // [3]: receivedProcess->state "started"
                    runningProcessPTR = &receivedProcess;
                    strcpy(runningProcessPTR->state, "started");
                    // [4]: wakeProccess(int runTime, const char* PROCESS_PATH)
                    runningProcessPTR->pID = wakeProccess(runningProcessPTR->runTime);
                    // [5]: Print to LogFile
                    printDEBUG(*runningProcessPTR);
                }
                else
                {
                    // Enqueue HPF, SRTN, RR, SJF FCFS Case
                    enqueue(&CPQptr, &receivedProcess, insertionFactor);
                }
            }
        }
    }
    //Delete Queue
    msgctl(receivingQueueID, IPC_RMID, NULL);
    printf("\nFinshed Schedular\n");
    
    destroyClk(false);
    exit(0);
}
