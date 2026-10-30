// expect: error lifetime @ L22
// mode: c23, c++20
// twin: tests/memory/ptrptr_set.c
// Pins: 0040 item 2 (audit N5): a callee stores the address of its local into the caller's cell; the contract says nothing of the stored pointer's liveness, so the caller's use of it (a test) is a lifetime error
#include <cerastes.h>
#include <stddef.h>

void escape(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp))
{
  int local = 1;
  *pp = &local;
}

int use_escape(void)
{
  int x = 0;
  int *p = &x;
  int **pp = &p;
  escape(pp);
  if (*pp != NULL) {
    x = 1;
  }
  return x;
}
