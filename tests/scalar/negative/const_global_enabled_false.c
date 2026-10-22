// expect: error postcondition @ L25
// mode: c23, c++20
// twin: tests/scalar/const_global.c
// Pins: 0017 item 3: a use of a constant is its value (ENABLED is false, so the function returns 0)
#include <cerastes.h>

const int LIMIT = 100;
const short STEP = 7;
const bool ENABLED = false;

int scaled_sum(int n)
    CERASTES_REQUIRES(0 <= n && n <= LIMIT)
    CERASTES_ENSURES(r, r == n * STEP)
{
  int i = 0;
  int total = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && total == i * STEP)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    CERASTES_CHECK(0 <= i);
    total += STEP;
    i += 1;
  }
  CERASTES_CHECK(total <= LIMIT * STEP);
  return total * ENABLED;
}

int use(void)
    CERASTES_ENSURES(r, r == 707)
{
  int a = scaled_sum(0);
  int b = scaled_sum(1);
  int c = scaled_sum(LIMIT);
  return a + b + c;
}
