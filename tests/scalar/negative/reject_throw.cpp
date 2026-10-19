// expect: rejected cerastes-unsupported_expression @ L9
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: throw is rejected (an expression of the verified function)
#include <cerastes.h>

int checked(int x)
{
  if (x < 0) {
    throw 1;
  }
  return x;
}
