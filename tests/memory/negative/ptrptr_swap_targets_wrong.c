// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/memory/ptrptr_swap_targets.c
// Pins: 0045 item 7: only *a is stored, so *b == old(*a) fails (postcondition at the closing brace)
#include <cerastes.h>
#include <stddef.h>

void swap_targets(int **a, int **b)
    CERASTES_REQUIRES(cerastes_owns(*a) && cerastes_owns(*b) && cerastes_owns(**a) && cerastes_owns(**b))
    CERASTES_ENSURES(cerastes_owns(*a) && cerastes_owns(*b) && *a == cerastes_old(*b) && *b == cerastes_old(*a) && cerastes_owns(**a) && cerastes_owns(**b) && **a == cerastes_old(**a))
{
  int *t = *a; *a = *b; *b = t;
}  // <- here
