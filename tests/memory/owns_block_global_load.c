// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 2 (owns_block(*gp), a place through a global pointer-to-pointer): put() gives the block stored in *gp (owns(gp) && owns(*gp) before the site, as 0045 item 3's chain), take() consumes it; main holds nothing at its end
// Pre-registered from record 0054 item 2 (forms "*gp", "g"; the site's loads are contract reads framed by owns(gp) and owns(*gp)): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

static int *cell = NULL;
static int **gp = &cell;

void put(void)
    CERASTES_REQUIRES(cerastes_owns(gp) && cerastes_owns(*gp))
    CERASTES_ENSURES(cerastes_owns(gp) && gp == cerastes_old(gp) && cerastes_owns(*gp) && cerastes_owns_block(*gp))
{
  *gp = (int *)malloc(sizeof(int));
  if (*gp == NULL) {
    abort();
  }
  **gp = 1;
}

void take(void)
    CERASTES_REQUIRES(cerastes_owns(gp) && cerastes_owns(*gp) && cerastes_owns_block(*gp))
    CERASTES_ENSURES(cerastes_owns(gp) && gp == cerastes_old(gp) && cerastes_owns(*gp))
{
  free(*gp);
  *gp = NULL;
}

int main(void)
{
  put();
  take();
  return 0;
}
