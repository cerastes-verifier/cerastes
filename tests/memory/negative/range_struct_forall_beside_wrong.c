// expect: error postcondition @ L36
// expect: error postcondition @ L46
// expect: error postcondition @ L55
// mode: c23, c++20
// twin: tests/memory/range_struct_forall_beside.c
// Pins: 0064 items 7, 9 and 11 (the vacuity twin of the control; the il-analyst's z08ok_bad shape): the same contracts with their own owns_range(a, m), and a body that writes a[0] = 5, which is not zero: the forall is not kept, a postcondition error once per function (at the return). It shows that the control's forall is read, not skipped
// Pre-registered from il-analyst-report-9 item 2 (z08ok_bad: postcondition) and the control, before any run: postcondition at the return of each function
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

CERASTES_PURE bool zero_at(int k, const int *p, int m)
{
  return !(0 <= k && k < m) || p[k] == 0;
}

int in_range(struct P *p, int n, int *a, int m)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && cerastes_owns_range(a, m) && n > 1 && m > 0 && cerastes_forall(zero_at, a, m) && p[1].x == 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && cerastes_owns_range(a, m) && cerastes_forall(zero_at, a, m) && p[1].x == 0)
{
  (void)p;
  (void)n;
  (void)m;
  a[0] = 5;
  return 0;
}

int in_member(struct B *b, int *a, int m)
    CERASTES_REQUIRES(cerastes_owns(*b) && cerastes_owns_range(a, m) && m > 0 && cerastes_forall(zero_at, a, m))
    CERASTES_ENSURES(r, cerastes_owns(*b) && cerastes_owns_range(a, m) && cerastes_forall(zero_at, a, m))
{
  (void)b;
  (void)m;
  a[0] = 5;
  return 0;
}

int in_global(int *a, int m)
    CERASTES_REQUIRES(cerastes_reads(g) && cerastes_owns_range(a, m) && m > 0 && cerastes_forall(zero_at, a, m))
    CERASTES_ENSURES(r, cerastes_reads(g) && cerastes_owns_range(a, m) && cerastes_forall(zero_at, a, m))
{
  (void)m;
  a[0] = 5;
  return 0;
}
