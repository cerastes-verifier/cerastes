// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 7 and 8 over 0060 item 12 (markers over a range of structures): a must_init filler of two elements writes both leaves of both (the flag of every leaf of the range holds on return), another forwards its own must_init range to it, and a ref_to_uninit callee keeps the flags of an initialized local array of structures
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: verifies
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
  p[1].y = 4;
}

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
