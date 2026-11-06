// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 6b (the former pin reject_owns_block_guard_global_ensures.c): the guard reads a global in an ensures (g > 0), with g == old(g); main frees the block it receives when it holds it
// Pre-registered from record 0052 item 6b: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

static int g = 1;

int *make(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(r, cerastes_owns(g) && g == cerastes_old(g) && (g > 0 ? cerastes_owns_block(r) : r == NULL))
{
  if (g > 0) {
    int *p = (int *)malloc(sizeof(int));
    if (p == NULL) {
      abort();
    }
    *p = g;
    return p;
  }
  return NULL;
}

int main(void)
{
  int *p = make();
  if (p != NULL) {
    free(p);
  }
  return 0;
}
