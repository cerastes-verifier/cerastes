// expect: rejected cerastes-unsupported_annotation @ L13
// mode: c23, c++20
// twin: tests/memory/struct_value_param.c
// Pins: 0044 item 3: owns(s.x) of a by-value parameter: the copy is the callee's, an atom on a member of it names no caller place
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int f(struct S s)
    CERASTES_REQUIRES(cerastes_owns(s.x))  // <- here
{
  return s.x;
}
