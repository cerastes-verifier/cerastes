// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 item 2: a loop that holds a pointer-typed node (a null test of a pointer to a local; the shape that 0032 item 10 rejected)
#include <cerastes.h>

int loop_pointer(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 0;
  int *p = &x;
  CERASTES_INVARIANT(0 <= n && n <= 100)
  CERASTES_DECREASES(n)
  while (n > 0) {
    if (p != 0) {
      n -= 1;
    }
  }
  return x;
}
