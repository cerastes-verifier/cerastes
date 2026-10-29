// expect: verified
// expect: native-ok
// mode: c23, c++20
// The invariant states the value of keep, so it survives the write through the other pointer.
// tag::source[]
#include <cerastes.h>

int havoc(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 7)
{
  int keep = 7;
  int last = 0;
  int *pk = &keep;
  int *pl = &last;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && keep == 7)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *pl = i;
    i += 1;
  }
  return *pk;
}
// end::source[]
