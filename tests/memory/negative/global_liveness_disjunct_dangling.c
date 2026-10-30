// expect: error lifetime @ L15
// mode: c23, c++20
// twin: tests/memory/global_liveness_disjunct.c
// Pins: 0041 item 6 (audit round 2, N1): without gp == &g nothing says what gp points to, and nothing held makes it live: `gp != NULL` is a lifetime error (the disjunct covers the globals' blocks only)
#include <cerastes.h>
#include <stddef.h>

static int g = 1;
static int *gp = &g;

int test(void)
    CERASTES_REQUIRES(cerastes_reads(gp))
    CERASTES_ENSURES(r, cerastes_reads(gp))
{
  return gp != NULL;
}

int main(void)
{
  (void)g;
  int r = test();
  return r;
}
