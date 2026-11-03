// expect: rejected cerastes-unsupported_declaration @ L12
// mode: c23, c++20
// twin: tests/memory/struct_value_param.c
// Pins: 0044 item 1 (M4.5f1): a pure function taking a struct
#include <cerastes.h>

struct S {
  int x;
  int y;
};

CERASTES_PURE int sq(struct S s)  // <- here
{
  return s.x;
}
