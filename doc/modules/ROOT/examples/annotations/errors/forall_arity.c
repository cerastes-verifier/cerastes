// expect: rejected cerastes-forall_argument_mismatch @ L13
// mode: c23, c++20
// cerastes_forall passes one argument for every parameter of the function after the first: the call is short by one.
// tag::bad[]
#include <cerastes.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

void use(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_forall(nonneg, a)) // <- error
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
}
// end::bad[]
