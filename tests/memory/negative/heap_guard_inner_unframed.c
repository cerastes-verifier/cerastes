// expect: error permission @ L9
// mode: c23, c++20
// twin: tests/memory/heap_guard_inner.c
// Pins: 0045 items 4, 11: the guard reads *pp before any atom frames it: permission at the contract clause
#include <cerastes.h>
#include <stddef.h>

void h(int **pp)
    CERASTES_REQUIRES((*pp == NULL || cerastes_owns(**pp)) && cerastes_owns(*pp))  // <- here
    CERASTES_ENSURES(cerastes_owns(*pp))
{
  (void)pp;
}
