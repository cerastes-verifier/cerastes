// expect: rejected bad_initializer_type @ L15
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 5, 0028 item 1: C's implicit incompatible pointer conversion (#144, an error in verify mode; C++ refuses it)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int implicit_pointer(void)
{
  struct S s = {1, 2};
  int *q = &s;  // <- here
  return q == &s.x;
}
