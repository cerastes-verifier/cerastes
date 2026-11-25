// expect: error permission @ L19
// mode: c23, c++20
// twin: tests/memory/struct_array_owns_array.c
// Pins: 0063 item 7: owns(p->a) is the array member alone: the write to p->n has no permission
// Pre-registered from record 0063 item 7 (Part C), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

void zero_first(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(p->a))
    CERASTES_ENSURES(cerastes_owns(p->a))
{
  p->a[0] = 0;
  p->n = 1;  // <- here
}
