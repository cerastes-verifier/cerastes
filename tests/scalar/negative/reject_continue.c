// expect: rejected cerastes-unsupported_statement @ L15
// mode: c23, c++20
// twin: tests/scalar/for_loop.c
// Pins: 0012 item 12: continue is rejected
#include <cerastes.h>

int count(int n)
    CERASTES_REQUIRES(n >= 0 && n <= 100)
{
  int i = 0;
  int total = 0;
  CERASTES_DECREASES(n - i)
  for (i = 0; i < n; i++) {
    if (i == 3) {
      continue;
    }
    total += 1;
  }
  return total;
}
