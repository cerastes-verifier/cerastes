// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 5 ("Calls", "main"): a consumer of owns(g) (a requires atom with no partner) takes the cell and the block with it; destroy frees the block, and main's held_g is false afterwards, so main's end needs no exemption
// Pre-registered from record 0054 item 5 (a consuming callee clears held_g without a read): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING static int *g = NULL;

void init(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  free(g);
  g = (int *)malloc(sizeof(int));
  if (g != NULL) {
    *g = 1;
  }
}

void destroy(void)
    CERASTES_REQUIRES(cerastes_owns(g))
{
  free(g);
}

int main(void)
{
  init();
  destroy();
  return 0;
}
