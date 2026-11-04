// expect: verified
// expect: native-ok
// mode: c23, c++20
// A loop: the invariant says what holds at every test, the measure shows it ends.
// tag::source[]
#include <cerastes.h>

int sum_to(int n)
    CERASTES_REQUIRES(0 <= n && n <= 1000)
    CERASTES_ENSURES(r, r == n * (n + 1) / 2)
{
  int i = 0;
  int s = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s == i * (i + 1) / 2)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
    s += i;
  }
  return s;
}
// end::source[]
