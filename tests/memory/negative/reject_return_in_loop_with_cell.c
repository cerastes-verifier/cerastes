// expect: rejected cerastes-unsupported_statement @ L16
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0032 item 10: a return inside a loop while a cell of an enclosing block is live is rejected (re-expected by b1c)
#include <cerastes.h>

int return_in_loop(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int x = 0;
  int *p = &x;
  CERASTES_INVARIANT(0 <= n && n <= 100)
  CERASTES_DECREASES(n)
  while (n > 0) {
    if (n == 50) {
      return 1;
    }
    n -= 1;
  }
  return *p;
}
