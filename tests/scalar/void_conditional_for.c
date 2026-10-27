// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0025 item 3: a void ?: as a for step, alone and in a comma; the step's calls run after each iteration, and their preconditions are checked there
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

int id(int a)
    CERASTES_ENSURES(r, r == a)
{
  return a;
}

// the step of a for loop is a void ?: (in a comma that advances the counter); pos accepts only m > 0, which c guarantees
int steps(int c, int n, int m)
    CERASTES_REQUIRES(0 <= n && n < 100 && (c == 0 || m > 0))
    CERASTES_ENSURES(r, r == n)
{
  int i;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  for (i = 0; i < n; (c ? pos(m) : (void)0), i++) {
  }
  return i;
}

// the whole step is the conditional; it runs after the body's i++, so pos(i) holds (a step run before the body would fail at i == 0)
int step_alone(int c, int n)
    CERASTES_REQUIRES(0 <= n && n < 100)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  for (; i < n; c ? pos(i) : (void)0) {
    i++;
  }
  return i;
}
