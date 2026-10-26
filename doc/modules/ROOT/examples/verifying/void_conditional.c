// expect: verified
// expect: native-ok
// mode: c23, c++20
// A conditional expression statement whose arms are void is the if it abbreviates, also as an operand of a comma whose value is unused and as the step of a for loop: each arm's precondition is checked on its own path.
// tag::source[]
#include <cerastes.h>

void pos(int x)
    CERASTES_REQUIRES(x > 0)
{
  (void)x;
}

void neg(int y)
    CERASTES_REQUIRES(y < 0)
{
  (void)y;
}

int choose(int c, int x, int y)
    CERASTES_REQUIRES(c ? x > 0 : y < 0)
    CERASTES_ENSURES(r, r == c)
{
  c ? pos(x) : neg(y);
  return c;
}

int count(int c, int n)
    CERASTES_REQUIRES(0 <= n && n < 100 && (c == 0 || n > 0))
    CERASTES_ENSURES(r, r == n)
{
  int i;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  for (i = 0; i < n; (c ? pos(n) : (void)0), i++) {
  }
  return i;
}
// end::source[]
