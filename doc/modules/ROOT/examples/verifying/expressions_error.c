// expect: error postcondition @ L18
// mode: c23, c++20
// The condition's i++ runs once more than the body does: after the loop i is n + 1, so the result is 3 * n, not 3 * n + 1.
// tag::source[]
#include <cerastes.h>

int count_up(int n)
    CERASTES_REQUIRES(0 <= n && n <= 715827882)
    CERASTES_ENSURES(r, r == 3 * n + 1)
{
  int i = 0;
  int total = 0;
  CERASTES_INVARIANT(i <= n && total == 2 * i)
  CERASTES_DECREASES(n - i)
  while (i++ < n) {
    total += 2;
  }
  return total + i - 1;
}
// end::source[]
