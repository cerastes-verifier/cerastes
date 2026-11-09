// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 2 (retires reject_owns_block_global.c's exact shape: a bodiless declaration f() with requires(owns(gp) && owns_block(gp)) on a pointer global): a bodiless consumer (a requires site alone is a consume, not a partnerless ensures site) is accepted, and main, which allocated gp's block, hands it over
// Pre-registered from record 0054 item 2 ("Bodiless contracts" concern ensures sites only): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

static int *gp = NULL;

void consume_gp(void)
    CERASTES_REQUIRES(cerastes_owns(gp) && cerastes_owns_block(gp));

int main(void)
{
  gp = (int *)malloc(sizeof(int));
  if (gp == NULL) {
    return 1;
  }
  *gp = 1;
  consume_gp();
  return 0;
}
