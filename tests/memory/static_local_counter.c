// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0041 item 12 (question 80): a static local of next is an implicit owns(count) in the contracts of next and of every function that reaches it (twice_in_order, the recursive ticks); main gets it from program_start; the value is unknown on each entry, so next resets it when out of range
#include <cerastes.h>

int next(void)
    CERASTES_ENSURES(r, 1 <= r && r <= 100)
{
  static int count = 0;
  if (count < 0 || count > 99) {
    count = 0;
  }
  count += 1;
  return count;
}

int twice_in_order(void)
    CERASTES_ENSURES(r, 2 <= r && r <= 200)
{
  int a = next();
  int b = next();
  return a + b;
}

void ticks(int n)
    CERASTES_REQUIRES(0 <= n && n <= 10)
    CERASTES_DECREASES(n)
{
  if (n > 0) {
    next();
    ticks(n - 1);
  }
}

int main(void)
{
  int a = next();
  CERASTES_CHECK(1 <= a && a <= 100);
  (void)a;
  ticks(3);
  int x = twice_in_order();
  int y = twice_in_order();
  return x - y;
}
