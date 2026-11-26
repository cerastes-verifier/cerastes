// expect: error postcondition @ L36
// expect: error postcondition @ L45
// expect: error postcondition @ L53
// mode: c23, c++20
// twin: tests/memory/range_struct_pure_beside.c
// Pins: 0064 items 7, 9 and 11 (the vacuity twin of the control, the il-analyst's m_g02ok shape): the same contracts with their own reads_range(a, m), and a body that returns a[0] + 1, which is 8 when first(a, m) == 7: the postcondition r == 7 fails at the return, once per function. It shows that the control's application is read, not skipped
// Pre-registered from il-analyst-report-9 item 1 and the control, before any run: postcondition at the return of each function
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  int y;
};

struct B {
  struct P v[3];
  int k;
};

struct P g[3];

CERASTES_PURE int first(const int *a, [[maybe_unused]] int m)
    CERASTES_REQUIRES(cerastes_reads_range(a, m) && m > 0)
{
  return a[0];
}

int in_range(struct P *p, int n, int *a, int m)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && cerastes_reads_range(a, m) && m > 0 && n > 0 && first(a, m) == 7)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && cerastes_reads_range(a, m) && r == 7)
{
  (void)p;
  (void)n;
  (void)m;
  return a[0] + 1;
}

int in_member(struct B *b, int *a, int m)
    CERASTES_REQUIRES(cerastes_owns(*b) && cerastes_reads_range(a, m) && m > 0 && first(a, m) == 7)
    CERASTES_ENSURES(r, cerastes_owns(*b) && cerastes_reads_range(a, m) && r == 7)
{
  (void)b;
  (void)m;
  return a[0] + 1;
}

int in_global(int *a, int m)
    CERASTES_REQUIRES(cerastes_reads(g) && cerastes_reads_range(a, m) && m > 0 && first(a, m) == 7)
    CERASTES_ENSURES(r, cerastes_reads(g) && cerastes_reads_range(a, m) && r == 7)
{
  (void)m;
  return a[0] + 1;
}
