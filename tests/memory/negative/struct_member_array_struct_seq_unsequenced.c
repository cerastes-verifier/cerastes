// expect: error unsequenced @ L46
// expect: error unsequenced @ L52
// expect: error unsequenced @ L58
// expect: error unsequenced @ L64
// mode: c23, c++20
// twin: tests/memory/struct_member_array_struct_seq.c
// Pins: 0064 items 11 and 12 (Part C2, audit round C1's finding 1 for an array member of structures; the sequencing rows of a call that owns an element, with no other check behind them): one function per row, each unsequenced at its expression, once. overlap: setP(&t.v[1]) twice (both own the element); own_and_read: setP(&t.v[1]) beside a read of t.v[1].x; whole_and_leaf: setT(&t), which owns every leaf of t, beside a read of t.v[0].x; own_and_value: setP(&t.v[1]) beside a by-value use of the element, takeP(t.v[1])
// Pre-registered from record 0064 items 11 and 12 and the audit's finding 1 (its batch s over a parameter's range, here over the array member), before any run: one unsequenced per function at the line of its expression
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

int setT(struct T *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && p->n == 1 && r == 0)
{
  p->n = 1;
  return 0;
}

int takeP(struct P t)
    CERASTES_ENSURES(r, r == t.x)
{
  return t.x;
}

int overlap(void)
{
  struct T t = {{{1, 2}, {3, 4}}, 5};
  return setP(&t.v[1]) + setP(&t.v[1]);
}

int own_and_read(void)
{
  struct T t = {{{1, 2}, {3, 4}}, 5};
  return setP(&t.v[1]) + t.v[1].x;
}

int whole_and_leaf(void)
{
  struct T t = {{{1, 2}, {3, 4}}, 5};
  return setT(&t) + t.v[0].x;
}

int own_and_value(void)
{
  struct T t = {{{1, 2}, {3, 4}}, 5};
  return setP(&t.v[1]) + takeP(t.v[1]);
}
