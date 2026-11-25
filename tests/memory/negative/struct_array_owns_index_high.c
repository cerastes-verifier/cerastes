// expect: error out-of-bounds @ L17
// mode: c23, c++20
// twin: tests/memory/struct_array_owns.c
// Pins: 0063 items 5 and 7: with i <= 4 the access p->a[i] may be one past the array: out-of-bounds (the staging pin's program)
// Pre-registered from record 0063 item 7 (Part C), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int f(struct S *p, int i)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= i && i <= 4)
{
  return p->a[i];  // <- here
}
