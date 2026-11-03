// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/memory/ptrptr_move.c
// Pins: 0045 items 3, 7: after a move **pp is q's object, whose value need not be the old **pp: postcondition at the closing brace
#include <cerastes.h>
#include <stddef.h>

void move(int **pp, int *q)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == q && cerastes_owns(**pp) && **pp == cerastes_old(**pp))
{
  *pp = q;
}  // <- here
