// expect: error out-of-bounds @ L24
// mode: c23, c++20
// twin: tests/memory/struct_array_value_elem.c
// Pins: 0063 item 13: `f().a[i]` is an element of a function's value with one out-of-bounds obligation: i == 4 is one past the array
// Pre-registered from record 0063 item 13 (Part E), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

struct S mk(int x)
    CERASTES_ENSURES(r, r.a[0] == x && r.a[1] == x && r.a[2] == x && r.a[3] == x && r.n == 0)
{
  struct S s = {{x, x, x, x}, 0};
  return s;
}

int pick(int i)
    CERASTES_REQUIRES(0 <= i && i <= 4)
{
  return mk(7).a[i];  // <- here
}
