// expect: error leak @ L29
// mode: c23, c++20
// verify-options: --strict-owner
// twin: tests/memory/struct_array_strict_owner.c
// Pins: 0064 item 15 under --strict-owner: the owning local keeps the structure's block and the function returns: leak at the return statement
// Pre-registered from record 0064 item 15 and the precedent owning_local_leak_return.c, before any run: leak at the return, under --strict-owner (unmeasured)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

CERASTES_OWNING struct S *make(void)
{
  CERASTES_OWNING struct S *a = (struct S *)calloc(1, sizeof(struct S));
  return a;
}

int use_owner(void)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_OWNING struct S *p = make();
  if (p != NULL) {
    p->a[1] = 3;
  }
  return 0;  // <- here
}
