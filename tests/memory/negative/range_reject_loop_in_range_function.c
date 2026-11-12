// expect: rejected cerastes-unsupported_annotation @ L13
// mode: c23, c++20
// twin: tests/memory/range_get_set.c
// Pins: STAGING PIN S2, deleted by Part D: a loop that touches memory in a function whose contract holds a range atom is rejected until the range places exist (record 0060 item 11; the tag and the line are a pre-registration: unsupported_annotation at the loop's line, attributes and while on one line: C++ positions an attributed statement at its first attribute)
// Pre-registered from record 0060 item 11 (staging pin S2), before the code
#include <cerastes.h>

void fill(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n >= 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n) CERASTES_DECREASES(n - i) while (i < n) {
    p[i] = 0;
    i++;
  }
}
