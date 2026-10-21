// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 item 5: the condition's obligations are checked where it is evaluated, after the body
// d is 0 before the body and 1 after it: the division in the condition is safe only after the body.
#include <cerastes.h>

int once_with_division(int n)
    CERASTES_REQUIRES(n <= 5)
    CERASTES_ENSURES(r, r == 1)
{
  int d = 0;
  int count = 0;
  CERASTES_INVARIANT(count == 0)
  CERASTES_DECREASES(1 - count)
  do {
    d = 1;
    count += 1;
  } while (n / d > 5);
  return count;
}
