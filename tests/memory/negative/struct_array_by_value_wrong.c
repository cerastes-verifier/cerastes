// expect: error check @ L27
// mode: c23, c++20
// twin: tests/memory/struct_array_by_value.c
// Pins: 0063 item 13: the callee changed its copy, not the caller's object: a check that the caller's s.a[1] changed fails
// Pre-registered from record 0063 item 13 (Part E), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int bump(struct S s)
    CERASTES_REQUIRES(s.a[1] == 2)
    CERASTES_ENSURES(r, r == 3)
{
  s.a[1] = s.a[1] + 1;
  return s.a[1];
}

int use(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct S s = {{1, 2, 3, 4}, 5};
  int v = bump(s);
  CERASTES_CHECK(s.a[1] == 3);  // <- here
  return s.n + v * 0;
}
