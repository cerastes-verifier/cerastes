// expect: rejected cerastes-unsupported_declaration @ L6
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: a consteval function is rejected
#include <cerastes.h>

consteval int pass_through(int x)
{
  return x;
}
