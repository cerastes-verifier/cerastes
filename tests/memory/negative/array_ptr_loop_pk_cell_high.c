// expect: error out-of-bounds @ L21
// mode: c23, c++20
// twin: tests/memory/array_ptr_loop_pk_cell.c
// Pins: 0059 item 4: the cell k is a one-slot block, so pk[1] is outside it: out-of-bounds at the store, in the loop
// Pre-registered from record 0059 item 4
#include <cerastes.h>

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 3)
    CERASTES_ENSURES(r, r == 0)
{
  int k = 0;
  int *pk = &k;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (i == n) {
      goto skip;
    }
    pk[1] = i;
  skip:
    i = i + 1;
  }
  return 0;
}
