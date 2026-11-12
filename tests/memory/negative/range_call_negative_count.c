// expect: error permission @ L84
// mode: c23, c++20
// twin: tests/memory/range_call_local.c
// Pins: 0060 item 8: a negative count: the shape row (n < 0) fails, one error at the call
// Pre-registered from record 0060 item 8 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

void zero_first(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[0] == 0)
{
  (void)n;
  p[0] = 0;
}

void zero_both(int *p, int n, int *q, int m)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_owns_range(q, m) && n > 0 && m > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_owns_range(q, m) && p[0] == 0 && q[0] == 0)
{
  (void)n;
  (void)m;
  p[0] = 0;
  q[0] = 0;
}

void touch(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  (void)p;
  (void)n;
}

int whole(void)
    CERASTES_ENSURES(r, r == 0)
{
  int a[4] = {1, 2, 3, 4};
  zero_first(a, 4);
  return a[0];
}

int sub(void)
    CERASTES_ENSURES(r, r == 0)
{
  int a[4] = {1, 2, 3, 4};
  zero_first(&a[1], 2);
  return a[1];
}

int twice(void)
    CERASTES_ENSURES(r, r == 0)
{
  int a[4] = {1, 2, 3, 4};
  zero_first(a, 4);
  zero_first(&a[0], 4);
  return a[0];
}

int two_ranges(void)
    CERASTES_ENSURES(r, r == 0)
{
  int a[4] = {1, 2, 3, 4};
  zero_both(&a[0], 2, &a[2], 2);
  return a[0] + a[2];
}

int filled(void)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_UNINIT int a[4];
  a[0] = 1;
  a[1] = 2;
  a[2] = 3;
  a[3] = 4;
  zero_first(a, 4);
  return a[0];
}

int empty(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a[4] = {1, 2, 3, 4};
  touch(a, -1);
  touch(&a[4], 0);
  return a[0];
}
