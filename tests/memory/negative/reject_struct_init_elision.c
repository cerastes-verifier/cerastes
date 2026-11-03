// expect: rejected cerastes-unsupported_expression @ L18
// mode: c23, c++20
// twin: tests/memory/struct_init_forms.c
// Pins: 0043 item 6: brace elision over a nested struct is rejected (clang's and GCC's -Wmissing-braces at -Wall: the union rule); C's {0} is the one exception
#include <cerastes.h>

struct S {
  int x;
  int y;
};
struct T {
  struct S t;
  int z;
};

int elided(void)
{
  struct T u = {1, 2, 3};  // <- here
  return u.z;
}
