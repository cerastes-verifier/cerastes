// expect: error null-dereference @ L17
// mode: c23, c++20
// twin: tests/memory/loop_null_or_owns.c
// Pins: 0038 item 3: the contract accepts NULL, the loop body does not guard the dereference
#include <cerastes.h>
#include <stddef.h>

int count_pos(int *p, int n)
    CERASTES_REQUIRES((p == NULL || cerastes_owns(*p)) && 0 <= n && n <= 100)
    CERASTES_ENSURES(r, (p == NULL || cerastes_owns(*p)) && 0 <= r && r <= n)
{
  int c = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && 0 <= c && c <= i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (*p > 0) {
      c += 1;
    }
    i += 1;
  }
  return c;
}
