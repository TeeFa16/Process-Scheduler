#include "headers.h"

int main(int agrc, char * argv[])
{
    printf("\nHELLO FROM PROCCESS.out MYID=%d, MYPPID=%d and Remaining Time = %s\n", getpid(), getppid(), argv[1]);
    // printf("\nBefore CLOCK = %ld\n", clock());
    // printf("\nCurrentTime: %ld\n", clock()/CLOCKS_PER_SEC);
    while (clock() < atoi(argv[1])*CLOCKS_PER_SEC);
    // printf("\nAfter CLOCK = %ld\n", clock());
    printf("\nI FINISHED with ID = %d\n", getpid());
    // printf("\nFinishTime: %ld\n", clock()/CLOCKS_PER_SEC);
    return 0;
}
