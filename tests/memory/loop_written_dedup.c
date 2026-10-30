// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 5 (audit B3): two reads atoms on one place (q == s): the later one takes nothing more only where the earlier one's guard holds; c == 0 makes it false
#include <cerastes.h>
#include <stddef.h>

int dedup(int *p, int c, int n)
    CERASTES_REQUIRES(cerastes_reads(*p) && 0 <= n && n <= 10)
    CERASTES_ENSURES(r, cerastes_reads(*p) && r == n)
{
  int *q = p;
  int *s = p;
  p = q;
  if (s == NULL) {
    return 0;
  }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (c == 0 || cerastes_reads(*q)) && cerastes_reads(*s))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
  (void)c;
  return i;
}
