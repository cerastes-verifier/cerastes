// expect: verified
// expect: native-ok
// mode: c23, c++20
// The same function, fixed: a pointer the function may read points into its object, never one past its end.
// tag::source[]
#include <cerastes.h>

bool same(const int *p, const int *q)
    CERASTES_REQUIRES(cerastes_reads(*p) && cerastes_reads(*q))
    CERASTES_ENSURES(cerastes_reads(*p) && cerastes_reads(*q))
{
  return p == q;
}
// end::source[]
