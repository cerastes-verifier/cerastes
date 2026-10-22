// expect: error uninitialized @ L10
// mode: c23, c++20
// twin: tests/scalar/uninit_compound.c
// Pins: 0019 item 2: x += reads x first (adds zero, so only the read can fail)
#include <cerastes.h>

unsigned int bump(unsigned int a)
{
  unsigned int x;
  x += 0u;
  return a;
}
