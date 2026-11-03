// expect: error uninitialized @ L20
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/struct_strict_init.c
// Pins: 0043 item 9, 0042 item 8: strict mode adds rejections and removes none: a marked struct local not written on every path is still read uninitialized
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int use_strict(int c)
{
  CERASTES_UNINIT struct S s;
  if (c != 0) {
    s.x = 1;
    s.y = 2;
  }
  return s.x;  // <- here
}
