// expect: rejected cerastes-unsupported_expression @ L14
// mode: c23, c++20
// twin: tests/memory/loop_pick_pointer.c
// Pins: 0038 items 6 and 10: &y in an annotation of a variable the code never takes the address of is no cell; rejected, not read
#include <cerastes.h>

int never_taken(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int x = 0;
  int y = 0;
  int *p = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p != &y)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
  return *p + y;
}
