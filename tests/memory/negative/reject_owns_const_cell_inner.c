// expect: rejected cerastes-unsupported_annotation @ L9
// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/owns_inner_const_cell.c
// Pins: 0045 item 13 (0040 item 2, P70 Q2): owns(*pp) with int *const *pp: the cell is const, rejected (owns(**pp) alone is accepted, owns_inner_const_cell.c)
#include <cerastes.h>

void set(int *const *pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp))  // <- here
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns(**pp));  // <- here

void use(void)
{
  int x = 0;
  int *const p = &x;
  set(&p);
}
