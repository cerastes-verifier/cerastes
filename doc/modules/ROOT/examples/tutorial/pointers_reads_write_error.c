// expect: error permission @ L12
// mode: c23, c++20
// The function writes *p, but reads gives it no right to write.
// Derived from pointers_reads.c.
// tag::source[]
#include <cerastes.h>

int get(int *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(r, cerastes_reads(*p) && r == *p)
{
  *p = 0;
  return *p;
}

int use_get(void)
    CERASTES_ENSURES(r, r == 9)
{
  int x = 9;
  int v = get(&x);
  CERASTES_CHECK(x == 9 && v == 9);
  return v;
}
// end::source[]
