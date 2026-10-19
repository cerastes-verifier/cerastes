// expect: rejected cerastes-unsupported_statement @ L9
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: an asm statement is rejected
#include <cerastes.h>

int barrier(int x)
{
  __asm__("");
  return x;
}
