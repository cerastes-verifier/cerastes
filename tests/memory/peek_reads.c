// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0037 items 6, 8, 10: reads(*p) fractions, called twice and the caller writes after; two reads atoms on one place (f(&x, &x)) with per-atom amounts
#include <cerastes.h>
#include <stddef.h>

int peek(const int *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(r, cerastes_reads(*p) && r == *p)
{
  return *p;
}

int two(const int *p, const int *q)
    CERASTES_REQUIRES(cerastes_reads(*p) && cerastes_reads(*q) && 0 <= *p && *p <= 1000 && 0 <= *q && *q <= 1000)
    CERASTES_ENSURES(r, cerastes_reads(*p) && cerastes_reads(*q) && r == *p + *q)
{
  return *p + *q;
}

int use_reads(void)
    CERASTES_ENSURES(r, r == 28)
{
  int x = 7;
  int a = peek(&x);
  int b = peek(&x);
  x = a + b;
  int s = two(&x, &x);
  return s;
}
