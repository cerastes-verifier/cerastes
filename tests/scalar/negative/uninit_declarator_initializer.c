// expect: error uninitialized @ L9
// mode: c23, c++20
// twin: tests/scalar/uninit_multi_declarator.c
// Pins: 0019 item 2: a later declarator's initializer reads the earlier one, which has no value
#include <cerastes.h>

int use(int a)
{
  int x, y = x;
  x = a;
  return y;
}
