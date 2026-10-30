// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0041 item 6 (audit round 2, N1): the liveness check of `gp != NULL` passes only through the disjunct lvloc(block_of(p)) == lvloc(GB_g()): the function reads gp and knows gp == &g, but holds no permission to g
#include <cerastes.h>
#include <stddef.h>

static int g = 1;
static int *gp = &g;

int test(void)
    CERASTES_REQUIRES(cerastes_reads(gp) && gp == &g)
    CERASTES_ENSURES(r, cerastes_reads(gp) && r == 1)
{
  return gp != NULL;
}

int main(void)
{
  int r = test();
  CERASTES_CHECK(r == 1);
  (void)r;
  return 0;
}
