// expect: rejected cerastes-unsupported_annotation @ L13
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 audit S4: a trusted contract that owns a member of a const struct pointee (it could write it) is rejected (M4.6)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void f(const struct S *p)
    CERASTES_REQUIRES(cerastes_owns(p->x))  // <- here
    CERASTES_ENSURES(cerastes_owns(p->x) && p->x == 2);

int ident(int a)
{
  return a;
}
