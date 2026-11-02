// expect: error permission @ L20
// mode: c23, c++20
// twin: tests/memory/struct_owns_nested.c
// Pins: 0043 item 9: owns(p->t) does not cover p->z
#include <cerastes.h>

struct S {
  int x;
  int y;
};
struct T {
  struct S t;
  int z;
};

void set_z(struct T *p)
    CERASTES_REQUIRES(cerastes_owns(p->t))
    CERASTES_ENSURES(cerastes_owns(p->t))
{
  p->z = 1;  // <- here
}
