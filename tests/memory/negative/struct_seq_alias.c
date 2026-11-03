// expect: error unsequenced @ L17
// mode: c23, c++20
// twin: tests/memory/struct_seq_distinct.c
// Pins: 0043 item 12: q may be p: p->x and q->x may be one object, and the read is unsequenced with the write
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int pick(struct S *p, struct S *r, int c)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*r) && -100 < p->x && p->x < 100)
    CERASTES_ENSURES(res, cerastes_owns(*p) && cerastes_owns(*r))
{
  struct S *q = c ? p : r;
  return p->x + (q->x = 1);  // <- here
}
