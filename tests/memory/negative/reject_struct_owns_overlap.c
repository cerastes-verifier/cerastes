// expect: rejected cerastes-unsupported_annotation @ L13
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 9, audit B1: owns(*p) && owns(p->x) holds one cell twice: rejected
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void f(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(p->x))  // <- here
{
}
