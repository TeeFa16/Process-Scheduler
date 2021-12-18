#include "headers.h"
//TEST CASES:
// #id arrival runtime priority
// 1	4	80	1
// 2	11	5	3
// 3	13	5	7
// TWO RR:
// 1	10	50	2
// 2	30	20	1
// 3	80	30	0
// 4	90	25	3
// 5	100	30	4
// #id arrival runtime priority
// 1	10	50	2
// 2	30	20	1
// 3	80	30	0
// 4	90	25	3
// 5	95	30	4

enum queueInsertionKey insertionFactor;
struct customPriorityQueue* CPQptr = NULL;
struct process* runningProcessPTR = NULL;
bool stillSending = true;
const char* PROCESS_PATH = NULL;
struct process runningProcess;
int startedProcessingTime = 0;
int QUANTUM;
FILE* logFile;

void printDEBUG(struct process* p)
{
    if(p == NULL)
    {
        return;
    }
    printf("\nAt\ttime\t%d\tprocess\t%d\t%s\tarr\t%d\tpriority\t%d\tremain\t%d\n", getClk(), p->id, p->state, p->arrivalTime, p->priority, p->remainingTime);
    // print in log file
    if (!strcmp(p->state, "finished"))
    {
        printf("\nTA = %d and WTA = %f\n", p->turnAround, p->weightedTurnAround);
        fprintf(logFile, "\nAt\ttime\t%d\tprocess\t%d\t%s\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%.2f\n", getClk(), p->id, p->state, p->arrivalTime, p->runTime, p->remainingTime, p->waitingTime, p->turnAround, p->weightedTurnAround);
    }
    else 
    {
        fprintf(logFile, "\nAt\ttime\t%d\tprocess\t%d\t%s\tarr\t%d\tremain\t%d\twait\t%d\n", getClk(), p->id, p->state, p->arrivalTime, p->remainingTime, p->waitingTime);
    }
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
    runningProcessPTR->remainingTime = runningProcessPTR->remainingTime - (getClk()-startedProcessingTime);
    runningProcessPTR->finishedTime = getClk();
    runningProcessPTR->waitingTime = runningProcessPTR->finishedTime - runningProcessPTR->arrivalTime - runningProcessPTR->runTime;
    runningProcessPTR->turnAround = runningProcessPTR->finishedTime - runningProcessPTR->arrivalTime;
    runningProcessPTR->weightedTurnAround = (float)runningProcessPTR->turnAround / runningProcessPTR->runTime;
    printf("\nFINISHED LOG\n");
    printProcess(runningProcessPTR);
    // TODO: store for analytics
    // Print to LOG file
    printDEBUG(runningProcessPTR);
    runningProcessPTR = dequeue(&CPQptr);
    if(runningProcessPTR == NULL)
    {
        return;
    }
    runningProcess = *runningProcessPTR;
    if(!strcmp(runningProcessPTR->state, "ready"))
    {
        strcpy(runningProcessPTR->state, "started");
        runningProcessPTR->waitingTime = getClk() - runningProcessPTR->arrivalTime;
        runningProcessPTR->pID = wakeProccess(runningProcessPTR->runTime);
    }
    else
    {
        strcpy(runningProcessPTR->state, "resumed");
        kill(runningProcessPTR->pID, SIGCONT);
    }
    // TODO:
    // Print to LOG file 
    startedProcessingTime = getClk();
    printDEBUG(runningProcessPTR);
    if (insertionFactor == RR)
        sleep(QUANTUM);
}


int main(int argc, char * argv[])
{
    // open log file
    logFile = fopen("scheduler.log","a");
    fprintf(logFile, "\nAt\ttime\tx\tprocess\ty\tstate\tarr\tw\ttotal\tz\tremain\ty\twait\tk\n");
    signal(SIGUSR1, schedularHandler);
    struct sigaction action;
    action.sa_handler = sigChildHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &action, 0) == -1)
    {
        perror("sigaction");
        exit(1);
    }
    initClk();
    insertionFactor = atoi(argv[1]);
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
        QUANTUM = atoi(argv[2]);
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
    PROCESS_PATH = strcat(processBuffer, "/process.out");
    // Retriveing receivingQueueID
    key_t keyID;
    keyID = ftok("keyfile", QUEUE_KEY);
    int receivingQueueID = msgget(keyID, 0666| IPC_CREAT);
    // The Holy While "Schedular Implementation"
    CPQptr = newCustomPriorityQueue();
    bool RRFirstTime = true;
    while(stillSending || runningProcessPTR!=NULL)
    {
        if(!stillSending && insertionFactor != RR)
        {
            sleep(1);
        }
        else 
        {
            struct msgbuff receivedProccessMSG;
            int rec_val;
            if (insertionFactor == RR && !RRFirstTime)
                rec_val = msgrcv(receivingQueueID, &receivedProccessMSG, sizeof(struct process), 7, IPC_NOWAIT);
            else
            {
                RRFirstTime = false;
                rec_val = msgrcv(receivingQueueID, &receivedProccessMSG, sizeof(struct process), 7, !IPC_NOWAIT);
            }
            if(rec_val != -1)
            {
                // intialize process struct
                struct process receivedProcess = receivedProccessMSG.p;
                strcpy(receivedProcess.state, "ready");
                receivedProcess.waitingTime = 0;
                receivedProcess.remainingTime = receivedProcess.runTime;
                printDEBUG(&receivedProcess);
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
                    runningProcess = receivedProcess;
                    runningProcessPTR = &runningProcess;
                    strcpy(runningProcessPTR->state, "started");
                    receivedProcess.waitingTime = getClk() - receivedProcess.arrivalTime;
                    runningProcessPTR->pID = wakeProccess(runningProcessPTR->runTime);
                    startedProcessingTime = getClk();
                    // Print to LOG file
                    printDEBUG(runningProcessPTR);
                }
                // RECIEVED::CASE 2
                else if(runningProcessPTR!=NULL)
                {
                    // SRTN Case
                    //Update Current Running Process remaining time
                    if(insertionFactor==SRTN)
                    {
                        runningProcessPTR->remainingTime = runningProcessPTR->remainingTime - (getClk()-startedProcessingTime);
                        startedProcessingTime = getClk();
                    }
                    if(insertionFactor==SRTN && runningProcessPTR->remainingTime>receivedProcess.remainingTime)
                    {
                        // TODO: replace runningProcess with receivedProcess
                        // [1]: stop runningProcess
                        strcpy(runningProcessPTR->state, "stopped");
                        kill(runningProcessPTR->pID, SIGSTOP);
                        runningProcessPTR->remainingTime = runningProcessPTR->remainingTime - (getClk()-startedProcessingTime);
                        enqueue(&CPQptr, *runningProcessPTR, insertionFactor);
                        // [2]: Print to LogFile
                        printDEBUG(runningProcessPTR);
                        // [3]: receivedProcess->state "started"
                        runningProcess = receivedProcess;
                        runningProcessPTR = &runningProcess;
                        strcpy(runningProcessPTR->state, "started");
                        runningProcessPTR->waitingTime = getClk() - runningProcessPTR->arrivalTime;
                        // [4]: wakeProccess(int runTime, const char* PROCESS_PATH)
                        runningProcessPTR->pID = wakeProccess(runningProcessPTR->runTime);
                        startedProcessingTime = getClk();
                        // [5]: Print to LogFile
                        printDEBUG(runningProcessPTR);
                    }
                    else
                    {
                        // Enqueue HPF, SRTN, RR, SJF FCFS Case
                        struct process toEnqueueProcess = receivedProcess;
                        enqueue(&CPQptr, toEnqueueProcess, insertionFactor);
                    }
                }
            }
            if (insertionFactor == RR)
            {
                // swap
                if(!isEmpty(&CPQptr))
                {
                    strcpy(runningProcessPTR->state, "stopped");
                    kill(runningProcessPTR->pID, SIGSTOP);
                    runningProcessPTR->remainingTime = runningProcessPTR->remainingTime - (getClk()-startedProcessingTime);
                    enqueue(&CPQptr, *runningProcessPTR, insertionFactor);
                    // [2]: Print to LogFile
                    printDEBUG(runningProcessPTR);
                    // [3]: receivedProcess->state "started"
                    runningProcessPTR = dequeue(&CPQptr);
                    if(runningProcessPTR != NULL)
                    {
                        runningProcess = *runningProcessPTR;
                        if(!strcmp(runningProcessPTR->state, "ready"))
                        {
                            strcpy(runningProcessPTR->state, "started");
                            runningProcessPTR->pID = wakeProccess(runningProcessPTR->runTime);
                            runningProcessPTR->waitingTime = getClk() - runningProcessPTR->arrivalTime;
                        }
                        else
                        {
                            strcpy(runningProcessPTR->state, "resumed");
                            runningProcessPTR->waitingTime = getClk() - runningProcessPTR->arrivalTime - (runningProcessPTR->runTime - runningProcessPTR->remainingTime);
                            kill(runningProcessPTR->pID, SIGCONT);
                        }
                        // TODO:
                        // Print to LOG file 
                        startedProcessingTime = getClk();
                        printDEBUG(runningProcessPTR);
                    }
                }
                // Sleeping
                sleep(QUANTUM);
            }
        }
    }
    //Delete Queue
    msgctl(receivingQueueID, IPC_RMID, NULL);
    printf("\nFinshed Schedular\n");
    
    //close log file
    fclose(logFile);

    destroyClk(false);
    exit(0);
}
