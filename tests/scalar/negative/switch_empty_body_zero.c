// expect: error division-by-zero @ L12
// mode: c23, c++20
// twin: tests/scalar/switch_empty_body.c
// Pins: 0014 item 6: x / y in the scrutinee of an empty switch may divide by zero
// y != -1 keeps INT_MIN / -1 out, so only y == 0 is reported.
#include <cerastes.h>

int ignore(int x, int y)
    CERASTES_REQUIRES(y != -1)
    CERASTES_ENSURES(r, r == x)
{
  switch (x / y) {
  }
  return x;
}
