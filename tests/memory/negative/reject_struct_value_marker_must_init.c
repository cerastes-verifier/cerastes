// expect: rejected cerastes-unsupported_annotation @ L12
// mode: c23, c++20
// twin: tests/memory/struct_value_param.c
// Pins: 0044 item 3: must_init on a by-value struct parameter: it is not a pointer
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int f(CERASTES_MUST_INIT struct S s)  // <- here
{
  return s.x;
}
