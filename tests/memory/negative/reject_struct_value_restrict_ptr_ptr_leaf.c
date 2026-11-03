// expect: rejected cerastes-unsupported_annotation @ L16
// c-only: restrict and compound literal objects do not exist in C++
// twin: tests/memory/struct_local.c
// Pins: 0044 item 11 (0037 item 2): a restrict int * beside a by-value struct with an int ** leaf: a pointer passed in it is not based on the restrict parameter, rejected
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct Q {
  int **q;
};

int f(int *restrict p, struct Q w)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  *p = 1;
  return w.q == 0;
}
