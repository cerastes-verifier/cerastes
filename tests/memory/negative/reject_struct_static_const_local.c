// expect: rejected cerastes-unsupported_declaration @ L14
// mode: c23, c++20
// twin: tests/memory/struct_static_local.c
// Pins: 0044 item 9 (M4.6): a static const struct local is a const object
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int f(void)
{
  static const struct S k = {1, 2};  // <- here
  return k.x;
}
