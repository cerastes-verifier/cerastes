// expect: rejected cerastes-self_assignment @ L9
// mode: c23, c++20
// twin: tests/scalar/bit_compound.c
// Pins: 0023 item 5, extension: x |= x (clang -Wself-assign)
#include <cerastes.h>

int same(int x)
{
  x |= x;
  return x;
}
