// expect: rejected cerastes-unsupported_annotation @ L11
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/struct_local.c
// Pins: 0043 item 10, audit B2: a restrict parameter beside a struct whose pointer member may be p
#include <cerastes.h>

struct Q {
  int *q;
};

int f(int *restrict p, struct Q *s)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*s) && s->q == p)
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(*s) && r == 2)
{
  *p = 1;
  *s->q = 2;
  return *p;
}
