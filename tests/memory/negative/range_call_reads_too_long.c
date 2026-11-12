// expect: error permission @ L31
// mode: c23, c++20
// twin: tests/memory/range_call_reads.c
// Pins: 0060 item 8: a read range one element longer than the array (count 4 on three elements) to a reads_range callee: the shape row fails at the call
// Pre-registered from record 0060 item 8 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

int peek(const int *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && r == p[0])
{
  (void)n;
  return p[0];
}

int peek2(const int *p, int n, const int *q, int m)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && cerastes_reads_range(q, m) && n > 0 && m > 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && cerastes_reads_range(q, m) && r == p[0])
{
  (void)n;
  (void)q;
  (void)m;
  return p[0];
}

int same(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a[3] = {1, 2, 3};
  return peek2(a, 4, a, 3);
}

int kept(void)
    CERASTES_ENSURES(r, r == 7)
{
  int a[3] = {7, 2, 3};
  int x = peek(a, 3);
  CERASTES_CHECK(a[0] == 7 && a[1] == 2);
  return x;
}

int whole_r(const int *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n))
{
  return peek(p, n);
}

int shorter_r(const int *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n > 1)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n))
{
  return peek(p, n - 1);
}

int tail_r(const int *p, int n, int k)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && 0 <= k && k < n)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n))
{
  return peek(&p[k], n - k);
}

int one_of_two(const int *p, int n, const int *q, int m)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && cerastes_reads_range(q, m) && n > 0 && m > 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && cerastes_reads_range(q, m))
{
  (void)q;
  (void)m;
  return peek(p, n);
}

int both_of_two(const int *p, int n, const int *q, int m)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && cerastes_reads_range(q, m) && n > 0 && m > 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && cerastes_reads_range(q, m))
{
  return peek2(p, n, q, m);
}
