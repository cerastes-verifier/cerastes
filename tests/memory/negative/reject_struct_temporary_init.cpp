// expect: rejected cerastes-unsupported_declaration @ L14
// mode: c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 audit S7: S s = S{1, 2} (a temporary copied into s) is not one of item 6's forms
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int temporary_init()
{
  S s = S{1, 2};  // <- here
  return s.x;
}
