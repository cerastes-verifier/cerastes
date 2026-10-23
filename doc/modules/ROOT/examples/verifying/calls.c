// expect: verified
// expect: native-ok
// mode: c23, c++20
// Calls inside expressions: a guarded call, a call in a loop condition, and two calls in one sum.
// tag::source[]
#include <cerastes.h>

int pos(int x)
    CERASTES_REQUIRES(x > 0)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int guarded(int a)
    CERASTES_ENSURES(r, r == (a > 1))
{
  return a > 0 && pos(a) > 1;
}

int next(int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == x + 1)
{
  return x + 1;
}

int count(int n)
    CERASTES_REQUIRES(0 <= n && n < 2147483647)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(i <= n)
  CERASTES_DECREASES(n - i)
  while (next(i) <= n) {
    i = i + 1;
  }
  return i;
}

int sum_two(int a, int b)
    CERASTES_REQUIRES(0 <= a && a <= 1000 && 0 <= b && b <= 1000)
    CERASTES_ENSURES(r, r == a + b + 2)
{
  return next(a) + next(b);
}
// end::source[]
