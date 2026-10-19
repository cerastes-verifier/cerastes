// expect: error division-by-zero @ L10
// mode: c23, c++20
// twin: tests/scalar/compound_div.c
// Pins: 0010 item 7: a /= b with b == 0, at the assignment
#include <cerastes.h>

int shrink(int a, int b)
    CERASTES_REQUIRES(b >= 0)
{
  a /= b;
  a %= b;
  return a;
}
