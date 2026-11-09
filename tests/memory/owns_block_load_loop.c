// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 2 ("Loops") and audit round 2 item 2 (probe l4): a requires site owns_block(*pp) through a load is an automatic loop place: the loop writes **pp, and the block is freed after it
// Pre-registered from record 0054 item 2 and the audit probe l4_loop_ok (verifies today): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

void f(int **pp, int n)
    CERASTES_REQUIRES(0 <= n && n <= 10 && cerastes_owns(*pp) && cerastes_owns_block(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    **pp = i;
    i += 1;
  }
  free(*pp);
}
