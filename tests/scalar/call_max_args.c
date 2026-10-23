// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021: sibling calls as arguments of a third call (max(f(a), f(b))); the third call's precondition and result use both
#include <cerastes.h>

int succ(int x)
    CERASTES_REQUIRES(0 <= x && x <= 1000)
    CERASTES_ENSURES(r, r == x + 1 && 1 <= r && r <= 1001)
{
  return x + 1;
}

int larger(int p, int q)
    CERASTES_ENSURES(r, r == (p > q ? p : q))
{
  return p > q ? p : q;
}

int test(int a, int b)
    CERASTES_REQUIRES(0 <= a && a <= 1000 && 0 <= b && b <= 1000)
    CERASTES_ENSURES(r, r == (a > b ? a : b) + 1)
{
  return larger(succ(a), succ(b));
}
