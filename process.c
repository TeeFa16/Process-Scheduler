#include "headers.h"

int main(int agrc, char * argv[])
{
    // printf("\nCurrentTime: %ld\n", clock()/CLOCKS_PER_SEC);
    while (clock() < atoi(argv[1])*CLOCKS_PER_SEC);
    // printf("\nFinishTime: %ld\n", clock()/CLOCKS_PER_SEC);
    return 0;
}
