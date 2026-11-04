// expect: verified
// expect: native-ok
// mode: c23, c++20
// A function that only reads takes reads: the value is the same after the call.
// tag::source[]
#include <cerastes.h>

int get(const int *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(r, cerastes_reads(*p) && r == *p)
{
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
