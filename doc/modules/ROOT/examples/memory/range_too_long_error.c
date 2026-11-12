// expect: error permission @ L18
// mode: c23, c++20
// The call asks for five elements of a four-element array: the range the callee needs is not all the caller's.
// tag::source[]
#include <cerastes.h>

void set(int *p, int n, int i, int v)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 <= i && i < n)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[i] == v)
{
  (void)n;
  p[i] = v;
}

void caller(void)
{
  int a[4] = {1, 2, 3, 4};
  set(a, 5, 0, 7);
}
// end::source[]
