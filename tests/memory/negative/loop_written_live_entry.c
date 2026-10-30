// expect: error permission @ L13
// mode: c23, c++20
// twin: tests/memory/loop_written_live.c
// Pins: 0040 item 5: nothing says p points to a live object, so the loop enters without the liveness its invariant needs
#include <cerastes.h>
#include <stddef.h>

int live_loop(int *p, int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_live(p))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
  return i;
}
