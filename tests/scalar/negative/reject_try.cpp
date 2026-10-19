// expect: rejected cerastes-unsupported_statement @ L8
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: try is rejected
#include <cerastes.h>

int guarded(int x)
{
  try {
    x += 1;
  } catch (...) {
    x = 0;
  }
  return x;
}
