// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0037 items 6, 8, 10: a callee passes its own fractions and its ownership on to callees, twice in a row
#include <cerastes.h>
#include <stddef.h>

#include <limits.h>
int peek(const int *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(r, cerastes_reads(*p) && r == *p)
{
  return *p;
}

int twice(const int *p)
    CERASTES_REQUIRES(cerastes_reads(*p) && *p >= 0 && *p <= 1000)
    CERASTES_ENSURES(r, cerastes_reads(*p) && r == 2 * *p)
{
  int a = peek(p);
  int b = peek(p);
  return a + b;
}

void inc(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < INT_MAX)
    CERASTES_ENSURES(cerastes_owns(*p) && *p == cerastes_old(*p) + 1)
{
  *p += 1;
}

void inc2(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < INT_MAX - 1)
    CERASTES_ENSURES(cerastes_owns(*p) && *p == cerastes_old(*p) + 2)
{
  inc(p);
  inc(p);
}

int use_chain(void)
    CERASTES_ENSURES(r, r == 14)
{
  int x = 5;
  inc2(&x);
  return twice(&x);
}
