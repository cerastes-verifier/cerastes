// expect: error check @ L32
// expect: error check @ L47
// expect: error check @ L62
// expect: error invariant-preserved @ L71
// mode: c23, c++20
// twin: tests/memory/struct_member_array_struct_loop.c
// Pins: 0064 item 10 (audit round C2, finding 3): a loop whose body writes through a pointer into the member array of a structure local does not keep the element's value: after the loop `t.v[1].x == 3` (a pointer to the cell, cell_alias), `t.v[0].y == 2` (a pointer to the element, elem_alias) and `t.v[1].y == 4` (a pointer to the structure, struct_alias) are not known, each a `check`; and the clause `p->v[1].x == 3` that the body breaks at i == 2 is not preserved (broken). One row per function, one reason each
// Registered from audit-C2 probes b14b, b14c, b14d (each measured on snapshot a044aff629b8: check at the check after the loop) and g03b d (invariant-preserved at the INVARIANT line), laid out in this file's own lines, before the first run on this build (the rows' lines are inferred from the layout, the categories measured)
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

int cell_alias(void)
{
  struct T t = {{{1, 2}, {3, 4}}, 5};
  int *q = &t.v[1].x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    *q = 50;
    i = i + 1;
  }
  CERASTES_CHECK(t.v[1].x == 3);  // <- here
  return t.n;
}

int elem_alias(void)
{
  struct T t = {{{1, 2}, {3, 4}}, 5};
  struct P *e = &t.v[0];
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    e->y = 50;
    i = i + 1;
  }
  CERASTES_CHECK(t.v[0].y == 2);  // <- here
  return t.n;
}

int struct_alias(void)
{
  struct T t = {{{1, 2}, {3, 4}}, 5};
  struct T *e = &t;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    e->v[1].y = 50;
    i = i + 1;
  }
  CERASTES_CHECK(t.v[1].y == 4);  // <- here
  return t.n;
}

int broken(struct T *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p->v[1].x == 3)
    CERASTES_ENSURES(r, cerastes_owns(*p) && p->v[1].x == 3 && r == 0)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && p->v[1].x == 3)  // <- here
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (i == 2) {
      p->v[1].x = 4;
    }
    i = i + 1;
  }
  return 0;
}
