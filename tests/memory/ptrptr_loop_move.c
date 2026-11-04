// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 10 (question 110): a move before the loop (int *prev = *pp; *pp = q;): the loop uses both objects, the entry object through the method-entry snapshot and q's object through the loop-entry snapshot
#include <cerastes.h>
#include <stddef.h>

void loop_move(int **pp, int *q, int n)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp) && cerastes_owns(*q) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_owns(**pp) && cerastes_owns(*q))
{
  int *prev = *pp;
  *pp = q;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && *pp == q)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *prev = i;
    **pp = i + 1;
    i += 1;
  }
  *pp = prev;
}
