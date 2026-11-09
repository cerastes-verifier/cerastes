// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 2 (retires reject_owns_block_global.c's shape, a requires site owns_block(gp) after owns(gp) on a pointer global): a non-owning pointer global given a block by install() (owns(g) before owns_block(g)) and consumed by remove_it(); main holds nothing at its end
// Pre-registered from record 0054 item 2 ("owns_block(g)", given then consumed): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

static int *g = NULL;

void install(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g) && cerastes_owns_block(g) && *g == 4)
{
  g = (int *)malloc(sizeof(int));
  if (g == NULL) {
    abort();
  }
  *g = 4;
}

void remove_it(void)
    CERASTES_REQUIRES(cerastes_owns(g) && cerastes_owns_block(g))
    CERASTES_ENSURES(cerastes_owns(g) && g == NULL)
{
  free(g);
  g = NULL;
}

int main(void)
{
  install();
  CERASTES_CHECK(*g == 4);
  remove_it();
  return 0;
}
