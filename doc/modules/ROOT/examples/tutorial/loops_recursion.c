// expect: verified
// expect: native-ok
// mode: c23, c++20
// The same sum by recursion: the function's measure decreases at the recursive call.
// tag::source[]
#include <cerastes.h>

int sum_to(int n)
    CERASTES_REQUIRES(0 <= n && n <= 1000)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, r == n * (n + 1) / 2)
{
  if (n == 0) {
    return 0;
  }
  return n + sum_to(n - 1);
}
// end::source[]
