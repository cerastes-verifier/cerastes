// expect: rejected cerastes-unsupported_declaration @ L12
// mode: c23, c++20
// twin: tests/memory/struct_value_param.c
// Pins: 0044 item 3 (M4.6): a const struct S parameter is a const object
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int f(const struct S s)  // <- here
{
  return s.x;
}
