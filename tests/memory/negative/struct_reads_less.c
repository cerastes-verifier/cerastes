// expect: error permission @ L15
// mode: c23, c++20
// twin: tests/memory/struct_reads_param.c
// Pins: 0043 item 9: reads(p->x) covers one leaf; reading p->y is not covered
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int pick(struct S *p)
    CERASTES_REQUIRES(cerastes_reads(p->x))
{
  return p->y;  // <- here
}
