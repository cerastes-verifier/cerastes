// expect: error postcondition @ L21
// mode: c23, c++20
// twin: tests/memory/range_struct_marker.c
// Pins: 0064 item 7 over 0060 item 12 (as range_marker_must_init_fail.c): the filler leaves p[1].y unwritten: it returns without initializing every leaf of p[0..2), which must_init promises: postcondition at the closing brace (the callers trust the contract)
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: postcondition at the closing brace of fill2
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  int y;
};

void fill2(struct P *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns_range(p, 2))
    CERASTES_ENSURES(cerastes_owns_range(p, 2) && p[0].x == 1)
{
  p[0].x = 1;
  p[0].y = 2;
  p[1].x = 3;
}  // <- here

void forward(struct P *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns_range(p, 2))
    CERASTES_ENSURES(cerastes_owns_range(p, 2) && p[0].x == 1)
{
  fill2(p);
}

void wr(struct P *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns_range(p, 2))
    CERASTES_ENSURES(cerastes_owns_range(p, 2))
{
  p[0].x = 7;
}

int keeps(void)
{
  struct P v[2] = {{1, 2}, {3, 4}};
  wr(v);
  return v[1].y;
}
