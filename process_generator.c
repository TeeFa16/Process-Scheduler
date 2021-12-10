#include "headers.h"

void clearResources(int);

int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);
    // 1. Read the input files.
    FILE* processesFile;
    char* processesFileName = argv[1];
    // count rows in file
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
    // start clk and schedular
    char clkBuffer[500];
    getcwd(clkBuffer, sizeof(clkBuffer));
    char schedularBuffer[500];
    getcwd(schedularBuffer, sizeof(schedularBuffer));
    const char* CLK_PATH = strcat(clkBuffer, "/clk.out");
    printf("\n clock path is : %s\n", CLK_PATH);
    const char* SCHEDULAR_PATH = strcat(schedularBuffer, "/scheduler.out");
    printf("\n scheduler path is : %s\n", SCHEDULAR_PATH);
    // ask user for input 
    int algorithmNum=1;
    int quantumNum;
    // printf("\nPlease enter algorithm number: \n");
    // scanf("%d", &algorithmNum);
    // if (algorithmNum == 3)
    // {
    //     printf("\nPlease enter quantum number: \n");
    //     scanf("%d", &quantumNum);
    // }
    // waking up clk
    const int CLK_PID = fork();
    if (CLK_PID == -1)
        return -1;
    else if (CLK_PID == 0)
    {
        printf("\n before clk  run\n");
        execl(CLK_PATH, "clk.out", NULL);
        printf("\na7aaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
    }

    const int SCHEDULAR_PID = fork();
    if (SCHEDULAR_PID == -1)
        return -1;
    else if (SCHEDULAR_PID == 0)
    {
        printf("\n before scheduler ID: %d\n", SCHEDULAR_PID);
        if (algorithmNum != 3)
        {
            printf("\n after algo not 3 schedule run\n");
            execl(SCHEDULAR_PATH, "scheduler.out", NULL);
            printf("\nerrrrrrrror\n");
        }
        else
        {
            printf("\n after algo 3 schedule run\n");
            execl(SCHEDULAR_PATH, "scheduler.out", '1', '1', NULL);
        }
    }

    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    // 3. Initiate and create the scheduler and clock processes.
    // 4. Use this function after creating the clock process to initialize clock
    initClk();
    // To get time use this
    int x = getClk();
    printf("\n current time is %d\n", x);
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    kill(SCHEDULAR_PID, SIGUSR1);
    printf("\n SCHEDULAR_PID: %d \n", SCHEDULAR_PID);
    int stat_loc;
    printf("\n Before wait proc_gen\n");
    int sid = wait(&stat_loc);
    printf("\n After wait proc_gen\n");
    while ((stat_loc & 0x00FF)) // wait until the scheduler exits or is destroyed
    {
        printf("\nDEBUG\n");
        int sid = wait(&stat_loc);
    };
    printf("\n finished\n");
    destroyClk(false);
    kill(CLK_PID, SIGINT);
    destroyClk(true);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    // FIX: interruptied
    killpg(getpgrp(), SIGKILL);
    destroyClk(true);
}
