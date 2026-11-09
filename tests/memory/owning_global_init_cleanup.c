// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 5 (retires reject_owning_global.c): an owning global (static, initial NULL) with init and cleanup functions whose contracts name only owns(g): the deep atom g == NULL || owns_block(g) comes with it in both lists, init frees the old block and allocates a new one, cleanup frees and sets g to NULL; main calls them in turn and ends with nothing held
// Pre-registered from record 0054 item 5 (S39 (b), tests/viper-smoke/memory/s39_b_owning_global.vpr): verifies in both modes
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

void cleanup(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  free(g);
  g = NULL;
}

int main(void)
{
  init();
  init();
  cleanup();
  init();
  cleanup();
  return 0;
}
