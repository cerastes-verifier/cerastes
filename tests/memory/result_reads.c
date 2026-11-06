// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 2: reads(*r) given: the callee keeps its requires reads(g) and returns the cell through *r; the caller reads it
// Pre-registered from record 0052 item 2: verifies in both modes
#include <cerastes.h>

static int g = 4;

int *view(void)
    CERASTES_REQUIRES(cerastes_reads(g))
    CERASTES_ENSURES(r, cerastes_reads(*r) && r == &g && *r == cerastes_old(g))
{
  return &g;
}

int main(void)
{
  int *a = view();
  int v = *a;
  CERASTES_CHECK(v == 4);
  return v - 4;
}
