// expect: rejected cerastes-uninit_unmarked @ L15
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/struct_strict_init.c
// Pins: 0043 item 9, 0042 item 8: --strict-init asks for the marker on every struct local without an initializer
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int use_strict(void)
{
  struct S s;  // <- here
  s.x = 1;
  s.y = 2;
  return s.x + s.y;
}
