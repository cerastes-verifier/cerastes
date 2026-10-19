// expect: rejected cerastes-unsupported_statement @ L12
// mode: c23, c++20
// twin: tests/scalar/while_invariant.c
// Pins: 0012 item 12: do ... while is rejected
#include <cerastes.h>

int count_to(int n)
    CERASTES_REQUIRES(n >= 0 && n < 1000)
{
  int i = 0;
  CERASTES_DECREASES(n - i)
  do {
    i += 1;
  } while (i < n);
  return i;
}
