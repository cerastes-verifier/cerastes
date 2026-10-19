// expect: golden
// expect: native-ok
// decreases: a loop's measure is written before the loop, a function's after its parameter list.
#include <cerastes.h>

// tag::loop[]
int count_up(int n) CERASTES_REQUIRES(n >= 0)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
  return i;
}
// end::loop[]

// tag::function[]
int count_down(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, r == 0)
{
  if (n == 0) {
    return 0;
  }
  return count_down(n - 1);
}
// end::function[]
