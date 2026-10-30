// expect: error permission @ L18
// mode: c23, c++20
// twin: tests/memory/ptrptr_set.c
// Pins: 0040 item 2 (0037 item 8): the caller passes a pointer cell it does not own, so set's requires owns(*pp) fails at the call
#include <cerastes.h>
#include <stddef.h>

void set(int **pp, int *q)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_live(q))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == q && cerastes_live(q))
{
  *pp = q;
}

void use_set(int **pp, int *q)
    CERASTES_REQUIRES(cerastes_live(q))
{
  set(pp, q);
}
