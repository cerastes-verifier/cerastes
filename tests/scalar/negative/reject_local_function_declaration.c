// expect: rejected cerastes-unsupported_declaration @ L9
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: a function declared inside a body is rejected
#include <cerastes.h>

int outer(int x)
{
  int helper(int);
  return x;
}
