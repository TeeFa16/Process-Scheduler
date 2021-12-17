#include "headers.h"

int main(int agrc, char * argv[])
{
    while (clock() < atoi(argv[1])*CLOCKS_PER_SEC);
    return 0;
}
