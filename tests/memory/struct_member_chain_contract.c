// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 3 (flip of reject_struct_member_chain_contract.c): p->q->x read in a contract once an atom owns it
#include <cerastes.h>
#include <stddef.h>
struct L {
  struct L *q;
  int x;
};

int f(struct L *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(p->q->x) && p->q->x > 0)
    CERASTES_ENSURES(r, cerastes_owns(*p) && p->q == cerastes_old(p->q) && cerastes_owns(p->q->x) && r == 1)
{
  (void)p;
  return 1;
}
