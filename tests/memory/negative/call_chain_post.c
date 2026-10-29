// expect: error postcondition @ L37
// mode: c23, c++20
// twin: tests/memory/call_chain.c
// Pins: 0037 item 10: inc2 promises +2 but calls inc once: the value ensures fails at the closing brace
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
}

int use_chain(void)
    CERASTES_ENSURES(r, r == 14)
{
  int x = 5;
  inc2(&x);
  return twice(&x);
}
