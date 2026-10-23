// expect: error uninitialized @ L11
// mode: c23, c++20
// twin: tests/scalar/uninit_if_else.c
// Pins: 0019 item 2: the condition of an if is a read
#include <cerastes.h>

int sign(int a)
{
  int x;
  int total = 0;
  if (x > 0) {
    total = a;
  }
  return total;
}
