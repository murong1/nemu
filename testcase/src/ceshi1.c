#include "trap.h"

int main()
{
    int  i;
   for( i=0;i<2;i++)
   {

   }
   nemu_assert(i == 0);
   nemu_assert(i == 1);
   nemu_assert(i == 2);
    nemu_assert(i == 3);
    nemu_assert(i == 4);

    return 0;
}