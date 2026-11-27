// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 3 and 7 (overlap is by slots): owns(*p) of a structure with an array member of structures and owns(*q) with q another object: disjoint, the call holds (the trap negative/struct_member_array_struct_alias_trap.c passes the address of an element of the member)
// Pre-registered from record 0064 items 3 and 7 (Part C2), before any run: verifies
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

void two(struct T *p, struct P *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns(*q) && p->n == 1 && q->x == 2)
{
  p->n = 1;
  q->x = 2;
}

int use(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct T t = {{{1, 2}, {3, 4}}, 5};
  struct P other = {0, 0};
  two(&t, &other);
  CERASTES_CHECK(t.n == 1 && other.x == 2);
  return t.n + other.x;
}
