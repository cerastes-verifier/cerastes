// expect: error unsequenced @ L17
// mode: c23, c++20
// twin: tests/memory/array_struct_sequencing.c
// Pins: 0064 item 11 (moved here from array_struct_reject_expressions.c, which pinned it as unsequenced_modification): p[i].x = p[i].x++ through a pointer is not the static unsequenced_modification, because a pointer base has no sequencing key (as scalar p[i] since M4.5e1a); it takes the path-precise unsequenced row at that line. The positive has no pointer function: a pointer base is path-precise even with distinct indexes, so this twin differs from the positive's named-array rows by the base alone
// Pre-registered from the record's revised item 11 (Part B), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int same_ptr(struct P *p, int i)
    CERASTES_REQUIRES(cerastes_owns(*p) && i == 0 && -100 < p->x && p->x < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  p[i].x = p[i].x++;  // <- here
  return 0;
}
