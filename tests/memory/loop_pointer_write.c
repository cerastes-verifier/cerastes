// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 items 2 and 3: a pointer to a local updated in a loop, the value in the invariant (the place keeps no value across a write through a pointer, the user's clause supplies it)
#include <cerastes.h>

int through_pointer(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == n)
{
  int x = 0;
  int *p = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && x == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p += 1;
    i += 1;
  }
  return x;
}
