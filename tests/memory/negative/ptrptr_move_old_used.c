// expect: error permission @ L19
// mode: c23, c++20
// twin: tests/memory/ptrptr_move.c
// Pins: 0045 item 7: the caller relies on the old object x after the call: the callee kept its cell (permission at the read)
#include <cerastes.h>
#include <stddef.h>

void move(int **pp, int *q)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == q && cerastes_owns(**pp) && **pp == cerastes_old(*q)) { *pp = q; }

int use_move(int *x)
    CERASTES_REQUIRES(cerastes_owns(*x))
    CERASTES_ENSURES(r, r == 5)
{
  int y = 5;
  int *p = x;
  move(&p, &y);
  return y + *x;  // <- here
}
