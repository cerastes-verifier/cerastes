// expect: rejected cerastes-unsupported_declaration @ L14
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 audit S4: a const struct local is rejected (const objects come with M4.6)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int const_local(void)
{
  const struct S s = {1, 2};  // <- here
  return s.x;
}
