// expect: rejected cerastes-forall_function @ L13
// mode: c23, c++20
// The function named by cerastes_forall must be a pure function: this one is an ordinary function.
// tag::bad[]
#include <cerastes.h>

bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

void use(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_forall(nonneg, a, n)) // <- error
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
}
// end::bad[]
