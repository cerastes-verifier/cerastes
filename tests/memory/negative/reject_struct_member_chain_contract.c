// expect: rejected cerastes-unsupported_annotation @ L13
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 15: p->q->x in a contract reads through a heap read (M4.5c2)
#include <cerastes.h>

struct L {
  struct L *q;
  int x;
};

void f(struct L *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p->q->x > 0)  // <- here
{
}
