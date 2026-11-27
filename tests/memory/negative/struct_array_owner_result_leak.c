// expect: error leak @ L29
// mode: c23, c++20
// twin: tests/memory/struct_array_owner_result.c
// Pins: 0064 item 15 (owning result and owning local): the caller keeps the block its owning local received and returns: leak at the return statement (the precedent negative/owning_local_leak_return.c)
// Pre-registered from record 0064 item 15 and the precedent owning_local_leak_return.c (leak at the return statement), before any run: unmeasured for an array leaf
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

CERASTES_OWNING struct S *make(void)
{
  struct S *p = (struct S *)calloc(1, sizeof(struct S));
  return p;
}

int use_and_forget(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_OWNING struct S *p = make();
  if (p == NULL) {
    return -1;
  }
  p->a[1] = 9;
  return 0;  // <- here
}
