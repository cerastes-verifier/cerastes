// expect: error termination @ L19
// mode: c23, c++20
// twin: tests/scalar/measure_lexicographic.c
// Pins: 0013 item 2: the components in the other order: n does not decrease when m does
// Same code, measure (n, m): the call lex(m - 1, -5) does not decrease n.
#include <cerastes.h>

int lex(int m, int n)
    CERASTES_REQUIRES(m >= 0)
    CERASTES_DECREASES(n, m)
    CERASTES_ENSURES(r, r == 0)
{
  if (m == 0) {
    return 0;
  }
  if (n > 0) {
    return lex(m, n - 1);
  }
  return lex(m - 1, -5);
}
