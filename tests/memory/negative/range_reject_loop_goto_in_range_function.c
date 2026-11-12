// expect: rejected cerastes-unsupported_annotation @ L14
// mode: c23, c++20
// twin: tests/memory/range_get_set.c
// Pins: STAGING PIN S2, deleted by Part D: a loop that touches no memory but leaves by an outward goto, in a function whose contract holds a range atom, is rejected until the range places exist (same tag and line as range_reject_loop_in_range_function.c)
// Pre-registered from record 0060 item 11 (staging pin S2, widened), before the code
#include <cerastes.h>

int find(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n >= 0)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n))
{
  int i = 0;
  (void)p;
  CERASTES_INVARIANT(0 <= i && i <= n) CERASTES_DECREASES(n - i) while (i < n) {
    if (i == 3) {
      goto done;
    }
    i++;
  }
done:
  return i;
}
