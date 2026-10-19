// expect: rejected cerastes-unsupported_attribute @ L8
// twin: tests/scalar/if_else.c
// Pins: 0011 item 7: an attribute outside the allow-list is rejected (assume is a promise nobody checks)
#include <cerastes.h>

int promise(int x)
{
  [[assume(x > 0)]];
  return x;
}
