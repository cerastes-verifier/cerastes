// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 11: contract arithmetic is checked for well-definedness, each clause under the clauses
// before it; every guard below is what the guard_dropped_* and guard_swapped_* tests drop or swap
#include <assert.h>
#include <cerastes.h>

int by_requires(int x)
    CERASTES_REQUIRES(x != 0 && 100 / x > 1)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int by_two_ensures(int x)
    CERASTES_REQUIRES(x > 0 && x <= 100)
    CERASTES_ENSURES(r, r != 0 && r <= 100)
    CERASTES_ENSURES(r, 100 / r >= 1)
{
  return x;
}

int by_overflow(int x)
    CERASTES_REQUIRES(x >= 0 && x < 2147483647)
    CERASTES_ENSURES(r, r < 2147483647 && r + 1 > x)
{
  return x;
}

int by_check(int x, int y)
    CERASTES_REQUIRES(x >= 0 && y == 0)
{
  CERASTES_CHECK(y == 0 || x / y >= 0);
  return x + y;
}

int by_assert(int x, int y)
    CERASTES_REQUIRES(x >= 0 && y == 0)
{
  assert(y == 0 || x / y >= 0);
  return x + y;
}

int by_requires_overflow(int x)
    CERASTES_REQUIRES(x == 2147483647 || x + 1 > 5)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int by_two_ensures_overflow(int x)
    CERASTES_REQUIRES(x >= 0 && x < 2147483647)
    CERASTES_ENSURES(r, r >= 0 && r < 2147483647)
    CERASTES_ENSURES(r, r + 1 > x)
{
  return x;
}

int by_invariant(int n)
    CERASTES_REQUIRES(n >= 1 && n <= 100)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (i == 0 || 100 / i >= 1))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
  return i;
}
