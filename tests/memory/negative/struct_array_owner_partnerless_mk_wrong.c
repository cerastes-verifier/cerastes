// expect: error check @ L20
// mode: c23, c++20
// twin: tests/memory/struct_array_owner_partnerless_mk.c
// Pins: 0064 item 15 (amends 0052 item 2): the same unguarded bodiless result atom as the positive; the caller's check that the element it wrote is 3 (it wrote 2) is false, at the check: the atom gives the cells at full permission and the element's value is what the caller wrote
// Pre-registered from record 0064 item 15, before any run: unmeasured (check at the check; the precedent negative/struct_array_heap_calloc_wrong.c)
#include <cerastes.h>

struct S {
  int a[4];
  int n;
};

struct S *mk(void)
    CERASTES_ENSURES(r, cerastes_owns(*r));

int call_mk(void)
{
  struct S *p = mk();
  p->a[1] = 2;
  CERASTES_CHECK(p->a[1] == 3);  // <- here
  return p->a[1];
}
