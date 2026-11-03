// expect: rejected cerastes-uninit_initialized @ L14
// mode: c23, c++20
// twin: tests/memory/struct_uninit_local.c
// Pins: 0043 item 9, 0042 item 3: uninit on a struct local with an initializer is rejected
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int use_uninit(void)
{
  CERASTES_UNINIT struct S s = {1, 2};  // <- here
  return s.x;
}
