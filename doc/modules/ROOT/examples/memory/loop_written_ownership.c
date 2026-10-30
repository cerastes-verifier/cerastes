// expect: verified
// expect: native-ok
// mode: c23, c++20
// A pointer the function assigns takes nothing into a loop by itself: the invariant says what the loop holds through q.
// tag::source[]
#include <cerastes.h>
#include <stddef.h>

int count(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p == 1 && 0 <= n && n <= 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == n)
{
  int *q = p;
  p = NULL;
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s == i && cerastes_owns(*q) && *q == 1)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s += *q;
    i += 1;
  }
  return s;
}
// end::source[]
