// expect: error permission @ L15
// mode: c23, c++20
// swap only reads *q (reads instead of owns), so its write to *q is not allowed.
// Derived from pointers_swap.c.
// tag::source[]
#include <cerastes.h>

void swap(int *p, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_reads(*q))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_reads(*q)
                     && *p == cerastes_old(*q) && *q == cerastes_old(*p))
{
  int t = *p;
  *p = *q;
  *q = t;
}

int use_swap(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a = 1;
  int b = 2;
  swap(&a, &b);
  return b;
}
// end::source[]
