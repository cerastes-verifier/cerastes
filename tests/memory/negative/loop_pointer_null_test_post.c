// expect: error postcondition @ L20
// mode: c23, c++20
// twin: tests/memory/loop_pointer_null_test.c
// Pins: 0038 item 2: the cell holds 0, not 1
#include <cerastes.h>

int loop_pointer(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 1)
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
