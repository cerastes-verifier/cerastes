// expect: rejected cerastes-unsequenced_modification @ L16
// mode: c23, c++20
// twin: tests/memory/struct_seq_distinct.c
// Pins: 0043 item 12 (P66 M6): p->x read and written: GCC -Wsequence-point (-Wall, both modes)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int arrow_same(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && -100 < p->x && p->x < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  return p->x + (p->x = 1);  // <- here
}
