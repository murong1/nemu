#include "trap.h"

int main()
{
    int loop[2]={1,2};
    int sum = loop[1]+loop[2];
    nemu_assert(sum == 3);
    return 0;
}