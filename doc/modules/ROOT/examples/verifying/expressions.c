// expect: verified
// expect: native-ok
// mode: c23, c++20
// Side effects inside expressions: the value of an assignment, x++ and ++x, a comma in a for loop, a loop condition with an effect.
// tag::source[]
#include <cerastes.h>

int store_and_use(int a)
    CERASTES_REQUIRES(-1073741824 <= a && a <= 1073741823)
    CERASTES_ENSURES(r, r == 2 * a + 1)
{
  int y;
  int z = (y = a) + 1;
  return z + y;
}

int both(int x)
    CERASTES_REQUIRES(-1073741824 <= x && x <= 1073741822)
    CERASTES_ENSURES(r, r == 2 * x + 2)
{
  int a = x++;
  int b = ++x;
  return a + b;
}

int meet(int n)
    CERASTES_REQUIRES(0 <= n && n < 2147483647)
    CERASTES_ENSURES(r, r == (n + 1) / 2)
{
  int i, j;
  CERASTES_INVARIANT(i <= j + 1 && i + j == n)
  CERASTES_DECREASES(j - i)
  for (i = 0, j = n; i < j; i++, j--) {
  }
  return i;
}

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
  return total + i;
}
// end::source[]
