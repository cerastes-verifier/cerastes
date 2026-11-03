// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 12 (question 99): reads(*q) for struct S *q as a written atom in an invariant
#include <cerastes.h>
#include <stddef.h>
struct S {
  int a;
  int b;
};

int sum(struct S *q, int n)
    CERASTES_REQUIRES(cerastes_reads(*q) && 0 <= q->a && q->a <= 10 && 0 <= n && n <= 100)
    CERASTES_ENSURES(r, cerastes_reads(*q) && 0 <= r && r <= 10 * n)
{
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && 0 <= s && s <= 10 * i && cerastes_reads(*q) && 0 <= q->a && q->a <= 10)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s += q->a;
    i += 1;
  }
  return s;
}
