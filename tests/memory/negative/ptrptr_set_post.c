// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/memory/ptrptr_set.c
// Pins: 0040 item 2: the contract says *pp keeps its old value, the body stores q
#include <cerastes.h>
#include <stddef.h>

void set(int **pp, int *q)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_live(q))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_live(q))
{
  *pp = q;
}
