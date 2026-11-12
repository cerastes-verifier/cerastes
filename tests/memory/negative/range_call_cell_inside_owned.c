// expect: error permission @ L39
// mode: c23, c++20
// twin: tests/memory/range_call_cell_beside.c
// Pins: 0060 item 8 (S44 (d)): an owns cell (&a[1]) inside an owned range (&a[0], 3) passed together: the separation row of a range and a cell fails at the call, one error
// Pre-registered from record 0060 item 8 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

void mix_r(const int *p, int n, const int *q)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && cerastes_reads(*q) && n > 0)
    CERASTES_ENSURES(cerastes_reads_range(p, n) && cerastes_reads(*q))
{
  (void)p;
  (void)n;
  (void)q;
}

void mix_o(int *p, int n, int *q)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_owns(*q))
{
  (void)p;
  (void)n;
  (void)q;
}

int reads_inside(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a[4] = {1, 2, 3, 4};
  mix_r(&a[0], 4, &a[1]);
  return a[0];
}

int own_disjoint(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a[4] = {1, 2, 3, 4};
  mix_o(&a[0], 3, &a[1]);
  return 1;
}
