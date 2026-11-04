// expect: error permission @ L9
// mode: c23, c++20
// twin: tests/memory/heap_guard_ternary.c
// Pins: 0045 item 11: the ?: condition reads *pp before any atom frames it: permission at the clause
#include <cerastes.h>
#include <stddef.h>

void h(int **pp)
    CERASTES_REQUIRES((*pp != NULL ? cerastes_owns(**pp) : 1) && cerastes_owns(*pp))  // <- here
    CERASTES_ENSURES(cerastes_owns(*pp))
{
  (void)pp;
}
