// expect: rejected cerastes-unsupported_statement @ L14
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0032 item 10: a loop that holds a pointer-typed node is rejected (re-expected by b1c)
#include <cerastes.h>

int loop_pointer(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
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
