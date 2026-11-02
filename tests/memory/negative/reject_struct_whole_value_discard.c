// expect: rejected cerastes-unsupported_expression @ L15
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 audit N3: a discarded whole-struct read is rejected
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int discard(void)
{
  struct S s = {1, 2};
  (void)s;  // <- here
  return s.x;
}
