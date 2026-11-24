// expect: error overflow @ L13
// mode: c23, c++20
// The overflow of a[i] + 1 is inside the pure function; it is reported at the cerastes_forall that uses it, with a note at the function.
// tag::source[]
#include <cerastes.h>

CERASTES_PURE bool grows(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] + 1 > a[i];
}

void use(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_forall(grows, a, n)) // <- error
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
}
// end::source[]
