// expect: rejected cerastes-unsupported_declaration @ L14
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 audit B1: an element of the initializer writes a leaf of the object being initialized: rejected
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int self_write(void)
{
  struct S s = {.x = 1, .y = (s.x = 6)};  // <- here
  return s.y;
}
