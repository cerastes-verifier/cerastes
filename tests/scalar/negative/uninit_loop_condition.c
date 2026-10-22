// expect: error uninitialized @ L14
// mode: c23, c++20
// twin: tests/scalar/uninit_do_while.c
// Pins: 0019 items 2 and 4: the condition of a loop is a read, checked on entry
#include <cerastes.h>

int count(int n)
    CERASTES_REQUIRES(1 <= n && n <= 100)
{
  int x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n && x <= 2147483647) {
    x = i;
    i += 1;
  }
  return i;
}
