// expect: error uninitialized @ L11
// mode: c23, c++20
// twin: tests/scalar/uninit_bool_enum.c
// Pins: 0019 item 1: a bool local read before it is assigned
#include <cerastes.h>

int pick(int a)
{
  bool b;
  int n = 0;
  if (b) {
    n = a;
  }
  return n;
}
