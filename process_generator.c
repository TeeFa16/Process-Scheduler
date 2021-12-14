#include "headers.h"

void clearResources(int);

int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);
    // 1. Read the input files.
    FILE* processesFile;
    char* processesFileName = argv[1];
    // count processes' rows in file
    int rowCount = 0;
    char c;
    processesFile = fopen(processesFileName, "r");
    c = fgetc(processesFile);
    while(c!=EOF)
    {
        if (c!='#')
            rowCount++;
        char dumBuffer[1024];
        fgets(dumBuffer, 1024, processesFile);
        c = fgetc(processesFile);
    }
    fclose(processesFile);
    // Start reading
    struct process inputProccesses[rowCount];
    int inputProccessesCount = 0;
    struct processInputLine processLineObj;
    processesFile = fopen(processesFileName, "r");
    c = fgetc(processesFile);
    while(c!=EOF)
    {
        if (c=='#')
        {
            char dumBuffer[1024];
            fgets(dumBuffer, 1024, processesFile);
        }
        else
        {
            processLineObj.processID = c - '0';
            fscanf(processesFile, "\t%d\t%d\t%d\n", &processLineObj.processArrTime, &processLineObj.processRunTime, &processLineObj.processPriority);
            struct process Obj;
            Obj.id = processLineObj.processID;
            Obj.arrivalTime = processLineObj.processArrTime;
            Obj.runTime = processLineObj.processRunTime;
            Obj.priority = processLineObj.processPriority;
            inputProccesses[inputProccessesCount++] = Obj;
        }
        c = fgetc(processesFile);
    }
    fclose(processesFile);
    // ask user for input 
    int algorithmNum;
    int quantumNum;
    printf("\nPlease enter algorithm number: \n");
    scanf("%d", &algorithmNum);
    if (algorithmNum == 3)
    {
        printf("\nPlease enter quantum number: \n");
        scanf("%d", &quantumNum);
    }
    if (algorithmNum < 1 || algorithmNum > 5)
    {
        printf("\nWrong algorithm number!\n");
        exit(-1);
    }
    // start clk and schedular
    char clkBuffer[500];
    getcwd(clkBuffer, sizeof(clkBuffer));
    char schedularBuffer[500];
    getcwd(schedularBuffer, sizeof(schedularBuffer));
    const char* CLK_PATH = strcat(clkBuffer, "/clk.out");
    const char* SCHEDULAR_PATH = strcat(schedularBuffer, "/scheduler.out");
    // waking up clk
    const int CLK_PID = fork();
    if (CLK_PID == -1)
        return -1;
    else if (CLK_PID == 0)
    {
        execl(CLK_PATH, "clk.out", NULL);
    }
    // waking up schedular
    const int SCHEDULAR_PID = fork();
    char algorithmNumChar[sizeof(int)];
    sprintf(algorithmNumChar, "%d", algorithmNum);
    if (SCHEDULAR_PID == -1)
        return -1;
    else if (SCHEDULAR_PID == 0)
    {
        if (algorithmNum != 3)
        {
            execl(SCHEDULAR_PATH, "scheduler.out", &algorithmNumChar, NULL);
        }
        else
        {
            char quantumNumChar[sizeof(int)];
            sprintf(quantumNumChar, "%d", quantumNum);
            execl(SCHEDULAR_PATH, "scheduler.out", &algorithmNumChar, &quantumNumChar, NULL);
        }
    }
    // Use this function after creating the clock process to initialize clock
    initClk();
    // To get time use this
    int x = getClk();
    printf("\nCurrent time is %d\n", x);
    //Creating Queue <process_gen, scheduler>
    key_t keyID = ftok("keyfile", 90);
    int sendingQueueID = msgget(keyID, 0666| IPC_CREAT);
    //Filling the Queue
    int fillingCounter = 0;
    if(sendingQueueID == -1)
    {
        perror("\nError in creating Queue...\n");
        exit(-1);
    }
    while (fillingCounter != inputProccessesCount)
    {
        int currentTime = getClk();
        int next_arrival_time = inputProccesses[fillingCounter].arrivalTime;
        int sleep_duration = next_arrival_time-currentTime;
        sleep(sleep_duration);
        printf("\nCurrentTime = %d, ArrivalTime = %d, slept for '%d' secs\n", currentTime, next_arrival_time, sleep_duration);
        // prepare massage buffer
        struct msgbuff send;
        send.p = inputProccesses[fillingCounter];
        send.mtype = 7;
        // send 
        int send_val = msgsnd(sendingQueueID, &send, sizeof(struct process), !IPC_NOWAIT);
        if(send_val == -1)
            perror("\nError in sending...\n");
        fillingCounter++;
    }
    // 7. Clear clock resources
    kill(SCHEDULAR_PID, SIGUSR1);
    int stat_loc;
    int sid = wait(&stat_loc);
    while ((stat_loc & 0x00FF)) // wait until the scheduler exits or is destroyed
    {
        int sid = wait(&stat_loc);
    };
    kill(CLK_PID, SIGINT);
    destroyClk(true);
}

void clearResources(int signum)
{
    printf("Clearing resources....\n");
    destroyClk(true);
    kill(getpid(), SIGKILL);
}
