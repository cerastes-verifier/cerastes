// expect: error null-dereference @ L21
// mode: c23, c++20
// twin: tests/memory/struct_array_owner_partnerless.c
// Pins: 0064 item 15 (0052 item 2a): the fresh result may be NULL (the contract says so): writing an element without the test is null-dereference (the precedent negative/result_bodiless_null.c)
// Pre-registered from record 0064 item 15 and the precedent result_bodiless_null.c (null-dereference at the write), before any run: unmeasured for an array leaf
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

struct S *mk(void)
    CERASTES_ENSURES(r, r == NULL || cerastes_owns(*r));

int use_fresh(void)
{
  struct S *p = mk();
  p->a[1] = 2;  // <- here
  return 0;
}
