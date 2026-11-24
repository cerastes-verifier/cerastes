// expect: rejected cerastes-unsupported_expression @ L14
// expect: rejected cerastes-unsupported_type @ L14
// mode: c23, c++20
// twin: tests/memory/owns_inc.c
// Pins: 0038 item 6 (staging pin of M4.5e1c; the construct stays rejected, M4.5e2 lifts it: record 0061 item 12, cut 1, question 260): cerastes_old of a by-value parameter in a loop invariant is rejected (two errors at the invariant: the conversion to a pointer type and the type `const void *` of cerastes_old_'s declaration): the function's entry value is not named inside a loop
// Pre-registered from M4.5e1c's Step 0 (R2), as the frozen build at bf820fe reports it
#include <cerastes.h>

int count_up(int n)
    CERASTES_REQUIRES(n >= 0 && n <= 100)
    CERASTES_ENSURES(r, r == cerastes_old(n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && n == cerastes_old(n))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i = i + 1;
  }
  return i;
}
