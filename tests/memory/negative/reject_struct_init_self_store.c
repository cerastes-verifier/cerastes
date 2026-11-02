// expect: rejected cerastes-unsupported_declaration @ L14
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 audit B1: a designated element writes another leaf of the object being initialized: rejected
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int self_store(void)
{
  struct S s = {.x = (s.y = 5)};  // <- here
  return s.y;
}
