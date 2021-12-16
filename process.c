#include "headers.h"

int main(int agrc, char * argv[])
{
    printf("\nHELLO FROM PROCCESS.out MYID=%d, MYPPID=%d\n", getpid(), getppid());
    // printf("\nCurrentTime: %ld\n", clock()/CLOCKS_PER_SEC);
    while (clock() < atoi(argv[1])*CLOCKS_PER_SEC);
    printf("\nI FINISHED\n");
    // printf("\nFinishTime: %ld\n", clock()/CLOCKS_PER_SEC);
    return 0;
}
