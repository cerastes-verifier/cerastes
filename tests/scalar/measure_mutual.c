// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0013 item 4: mutual recursion, each function with its own measure (the declaration repeats the contract)
#include <cerastes.h>

int is_odd(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, r == 0 || r == 1);

int is_even(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, r == 0 || r == 1)
{
  if (n == 0) {
    return 1;
  }
  return is_odd(n - 1);
}

int is_odd(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, r == 0 || r == 1)
{
  if (n == 0) {
    return 0;
  }
  return is_even(n - 1);
}
