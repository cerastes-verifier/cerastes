// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 11 and 12 (Part C2, audit round C1's finding 1 for an array member of structures): calls on the address of an element of t.v beside each other: setP(&t.v[0]) and setP(&t.v[1]) own different elements and are accepted, both writes are known afterwards; setP(&t.v[0]) beside a read of the other element t.v[1].x, and beside a read of t.n (a member the callee does not own), are accepted too. negative/struct_member_array_struct_seq_unsequenced.c moves the operand into the element the callee owns
// Pre-registered from record 0064 items 11 and 12 and the audit's finding 1, before any run: verifies
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

int setP(struct P *q)
    CERASTES_REQUIRES(cerastes_owns(*q))
    CERASTES_ENSURES(r, cerastes_owns(*q) && q->x == 5 && r == 0)
{
  q->x = 5;
  return 0;
}

int disjoint(void)
    CERASTES_ENSURES(r, r == 10)
{
  struct T t = {{{1, 2}, {3, 4}}, 5};
  int s = setP(&t.v[0]) + setP(&t.v[1]);
  return s + t.v[0].x + t.v[1].x;
}

int beside_element(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct T t = {{{1, 2}, {3, 4}}, 5};
  return setP(&t.v[0]) + t.v[1].x;
}

int beside_member(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct T t = {{{1, 2}, {3, 4}}, 5};
  return setP(&t.v[0]) + t.n;
}
