// expect: rejected cerastes-unsupported_annotation @ L11
// mode: c23, c++20
// twin: tests/memory/loop_owns_fill.c
// Pins: 0038 items 2 and 10: an atom in the invariant of a do ... while (0) (straight-line code, asserted like a check) is rejected (never lifted)
#include <cerastes.h>

void once(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  CERASTES_INVARIANT(cerastes_owns(*p))
  CERASTES_DECREASES(1)
  do {
    *p = 0;
  } while (0);
}
