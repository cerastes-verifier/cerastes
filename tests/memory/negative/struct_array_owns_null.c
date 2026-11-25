// expect: error null-dereference @ L19
// mode: c23, c++20
// twin: tests/memory/struct_array_owns.c
// Pins: 0063 items 5 and 7: p->a[1] through a pointer that may be null: the root's row of 0043 item 5, once, null-dereference (the access is not a use of p's value, so no liveness row)
// Pre-registered from record 0063 item 7 (Part C), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int f(int c)
    CERASTES_ENSURES(r, r == 0)
{
  struct S s = {{1, 2, 3, 4}, 5};
  struct S *p = c != 0 ? &s : NULL;
  p->a[1] = 9;  // <- here
  return s.n - 5;
}
