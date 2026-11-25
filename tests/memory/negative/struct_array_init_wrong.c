// expect: error check @ L18
// mode: c23, c++20
// twin: tests/memory/struct_array_init.c
// Pins: 0063 item 6: the zero fill gives s.a[3] the value 0, so the check that it is 1 fails
// Pre-registered from record 0063 item 6 (Part C), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int f(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct S partial = {{1, 2}, 5};
  CERASTES_CHECK(partial.a[3] == 1);  // <- here
  return partial.n - 5;
}
