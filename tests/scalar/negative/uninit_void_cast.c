// expect: error uninitialized @ L10
// mode: c23, c++20
// twin: tests/scalar/uninit_sizeof.c
// Pins: 0019 item 2: (void)x reads x (stricter than C++, where it is allowed for unsigned char only)
#include <cerastes.h>

int size_of(void)
{
  int x;
  (void)x;
  return (int)sizeof(x);
}
