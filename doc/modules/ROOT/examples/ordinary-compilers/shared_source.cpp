// expect: golden
// expect: native-ok
// One source, two builds: the contract and the invariant vanish for an ordinary compiler.
// tag::source[]
#include <cerastes.h>

int sum_to(int n) CERASTES_REQUIRES(n >= 0) CERASTES_ENSURES(r, r >= 0)
{
  int i = 0;
  int s = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s >= 0) while (i < n) {
    s += i;
    i += 1;
  }
  CERASTES_CHECK(s >= 0);
  return s;
}
// end::source[]
