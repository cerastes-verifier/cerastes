// expect: error postcondition @ L25
// mode: c23, c++20
// twin: tests/memory/struct_array_chain_owns.c
// Pins: 0064 item 15 (306): the callee's ensures claims p->q->a[1] == 1 and the body writes 0: postcondition at the closing brace
// Pre-registered from record 0064 item 15, before any run: postcondition at the closing brace of zero_one
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

struct R {
  struct S *q;
  int z;
};

void zero_one(struct R *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*p->q))
    CERASTES_ENSURES(cerastes_owns(*p) && p->q == cerastes_old(p->q) && cerastes_owns(*p->q) && p->q->a[1] == 1)
{
  p->q->a[1] = 0;
}  // <- here
