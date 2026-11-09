// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 5 (retires reject_owning_global.c's exact shape: external linkage, "= NULL"): a function that only reads the owning global writes reads(g) (here with the liveness of a non-null value, (g == NULL || live(g)), which comparing the pointer needs: 0032 item 6), which gives the value and no part of the block; main assigns g first because C++'s external-linkage value is unknown ([basic.start.dynamic]), then init, is_set and cleanup
// Pre-registered from record 0054 item 5 ("The initial value", "A function that only reads the global"): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING int *g = NULL;

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

int is_set(void)
    CERASTES_REQUIRES(cerastes_reads(g) && (g == NULL || cerastes_live(g)))
    CERASTES_ENSURES(r, cerastes_reads(g) && (g == NULL || cerastes_live(g)) && (r == 0 || r == 1))
{
  return g != NULL;
}

int main(void)
{
  g = NULL;
  init();
  int s = is_set();
  CERASTES_CHECK(s == 0 || s == 1);
  (void)s;
  cleanup();
  return 0;
}
