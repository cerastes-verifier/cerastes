// expect: rejected cerastes-unsupported_statement @ L8
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: an if with an init-statement is rejected (C++17)
#include <cerastes.h>

int test(int x)
{
  if (int y = x + 1; y > 0) {
    return y;
  }
  return 0;
}
