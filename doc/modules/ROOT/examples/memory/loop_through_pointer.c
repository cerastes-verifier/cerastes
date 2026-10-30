// expect: verified
// expect: native-ok
// mode: c23, c++20
// A pointer to a local updated in a loop: the loop keeps its permission to the local, and the invariant states the value.
// tag::source[]
#include <cerastes.h>

int count_up(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == n)
{
  int x = 0;
  int *p = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && x == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p += 1;
    i += 1;
  }
  return x;
}
// end::source[]
