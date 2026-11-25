// expect: error precondition @ L24
// mode: c23, c++20
// twin: tests/memory/struct_array_value_elem.c
// Pins: 0063 item 13: `s.a[i]` of a by-value parameter in a requires is the argument's element: the caller's z.a[1] is 0, so the call fails the precondition
// Pre-registered from record 0063 item 13 (Part E), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int get(struct S s, int i)
    CERASTES_REQUIRES(0 <= i && i < 4 && s.a[i] > 0)
    CERASTES_ENSURES(r, r == s.a[i])
{
  return s.a[i];
}

int use(void)
{
  struct S z = {{0, 0, 0, 0}, 0};
  return get(z, 1);  // <- here
}
