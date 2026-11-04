// expect: rejected cerastes-unsupported_annotation @ L9
// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/owns_inner_const_cell.c
// Pins: 0045 item 13 (0040 item 2): owns of a const place at its own level: owns(**pp) with const int **pp is rejected (the contract could write a const object)
#include <cerastes.h>

void set(const int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp))  // <- here
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_owns(**pp));  // <- here

void use(void)
{
  const int c = 1;
  const int *p = &c;
  set(&p);
}
