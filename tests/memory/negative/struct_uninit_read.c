// expect: error uninitialized @ L16
// mode: c23, c++20
// twin: tests/memory/struct_uninit_local.c
// Pins: 0043 item 9, 0042 item 3: the marker does not weaken 0019: the member s.y is read uninitialized
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int use_uninit(void)
{
  CERASTES_UNINIT struct S s;
  s.x = 1;
  return s.y;  // <- here
}
