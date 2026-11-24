// expect: rejected cerastes-unsupported_expression @ L14
// expect: rejected cerastes-unsupported_type @ L14
// mode: c23, c++20
// twin: tests/memory/range_old_elem.c
// Pins: 0038 item 6 (staging pin of M4.5e1c, record 0061 item 12, Flips A: retired by Part D for a verifying positive): cerastes_old of an element p[i], i the loop's local, in a loop invariant is rejected (two errors at the invariant: the conversion to a pointer type and the type `const void *` of cerastes_old_'s declaration): the function's entry value is not named inside a loop
// Pre-registered from M4.5e1c's Step 0 (R2), as the frozen build at bf820fe reports it
#include <cerastes.h>

void keep_tail(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n >= 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (i == n || p[i] == cerastes_old(p[i])))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i = i + 1;
  }
}
