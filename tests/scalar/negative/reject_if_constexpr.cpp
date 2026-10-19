// expect: rejected cerastes-unsupported_statement @ L8
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: if constexpr is rejected (it is evaluated at compile time)
#include <cerastes.h>

int pick(int x)
{
  if constexpr (sizeof(int) == 4) {
    x += 1;
  }
  return x;
}
