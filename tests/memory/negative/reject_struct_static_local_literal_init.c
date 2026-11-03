// expect: rejected cerastes-unsupported_declaration @ L14
// mode: c23, c++20
// twin: tests/memory/struct_static_local.c
// Pins: 0044 item 9 (a GNU extension): a static local initialized from a compound literal is rejected
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int f(void)
{
  static struct S s = (struct S){1, 2};  // <- here
  return s.x;
}
