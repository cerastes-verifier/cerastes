// expect: rejected cerastes-unsupported_annotation @ L12
// The C++ spelling cerastes::forall is not read by cerastes verify: the message says to write cerastes_forall.
// tag::bad[]
#include <cerastes.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

void use(const int *a, int n)
    CERASTES_REQUIRES(cerastes::reads_range(a, n) && cerastes::forall(nonneg, a, n)) // <- error
    CERASTES_ENSURES(cerastes::reads_range(a, n))
{
  (void)a;
  (void)n;
}
// end::bad[]
