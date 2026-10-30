// expect: error permission @ L19
// mode: c23, c++20
// twin: tests/memory/loop_written_dedup.c
// Pins: 0040 item 5 (audit B3): owns(*s) beside a reads(*q) atom whose guard may hold with q == s: the separation fails at the later atom
#include <cerastes.h>
#include <stddef.h>

int dedup(int *p, int c, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 10)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == n)
{
  int *q = p;
  int *s = p;
  p = q;
  if (s == NULL) {
    return 0;
  }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (c == 0 || cerastes_reads(*q)) && cerastes_owns(*s))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
  (void)c;
  return i;
}
