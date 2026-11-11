// expect: error out-of-bounds @ L18
// expect: error permission @ L18
// mode: c23, c++20
// twin: tests/memory/array_ptr_loop_kept.c
// Pins: 0059 items 4 and 10 (S43 N1): pk[1] in the loop with only owns(*p) behind pk: two errors at one line, the bound and the permission of the second slot (the twin that replaces the false-alarm pin)
// Pre-registered from record 0059 items 4 and 10
#include <cerastes.h>

void fill(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int *pk = p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    pk[1] = i;
    i = i + 1;
  }
}
