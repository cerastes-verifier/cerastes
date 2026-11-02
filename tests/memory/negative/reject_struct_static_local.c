// expect: rejected cerastes-unsupported_declaration @ L14
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 15: a static struct local is rejected (M4.5c2)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int counter(void)
{
  static struct S s;  // <- here
  s.x += 1;
  return s.x;
}
