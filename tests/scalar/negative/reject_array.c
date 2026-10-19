// expect: rejected cerastes-unsupported_declaration @ L9
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: an array is rejected
#include <cerastes.h>

int first(int a)
{
  [[maybe_unused]] int values[3] = {1, 2, 3};
  return a;
}
