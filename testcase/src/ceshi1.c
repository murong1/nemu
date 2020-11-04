#include "trap.h"

int main()
{
    int loop;
    loop = 2;
    loop+=2;
    nemu_assert(loop == 4);
    return 0;
}