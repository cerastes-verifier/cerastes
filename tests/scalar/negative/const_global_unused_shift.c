// expect: error shift @ L9
// mode: c23, c++20
// twin: tests/scalar/const_global.c
// Pins: 0017 item 4: every constant is checked whether or not anything uses it
#include <cerastes.h>

const int LIMIT = 100;
const short STEP = 7;
[[maybe_unused]] const int UNUSED = 1 << 40;
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
