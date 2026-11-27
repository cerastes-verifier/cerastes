// expect: error permission @ L23
// mode: c23, c++20
// twin: tests/memory/struct_member_array_struct_owns.c
// Pins: 0064 item 7: owns(p->v) is the array member alone: the write to p->n has no permission
// Pre-registered from record 0064 item 7 (Part C2), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

void zero_first(struct T *p)
    CERASTES_REQUIRES(cerastes_owns(p->v))
    CERASTES_ENSURES(cerastes_owns(p->v))
{
  p->v[0].x = 0;
  p->n = 1;  // <- here
}
