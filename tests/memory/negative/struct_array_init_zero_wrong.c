// expect: error check @ L18
// c-only: the twin of a C-only test
// twin: tests/memory/struct_array_init_zero.c
// Pins: 0063 item 6 (question 303): {0} zeroes the element, so the check that s.a[2] is 1 fails
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
  struct S s = {0};
  CERASTES_CHECK(s.a[2] == 1);  // <- here
  return s.n;
}
