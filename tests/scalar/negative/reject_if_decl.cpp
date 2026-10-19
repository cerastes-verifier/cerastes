// expect: rejected cerastes-unsupported_statement @ L8
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: a condition declaration is rejected
#include <cerastes.h>

int test(int x)
{
  if (int y = x) {
    return y;
  }
  return 0;
}
