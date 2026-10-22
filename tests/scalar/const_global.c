// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0017 items 1 and 3: a const global is a named constant; each use is a literal of its type holding the initializer's value
// The constants are read in the body, in requires and ensures, in a check and in a loop invariant and measure;
// use() calls scaled_sum at both ends of its range.
#include <cerastes.h>

const int LIMIT = 100;
const short STEP = 7;
const bool ENABLED = true;

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
