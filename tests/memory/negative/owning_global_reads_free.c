// expect: error invalid-free @ L41
// mode: c23, c++20
// twin: tests/memory/owning_global_reads.c
// Pins: 0054 item 5: reads(g) gives g's value and no part of the block (no token): a function with only reads(g) cannot free it
// Pre-registered from record 0054 item 5 ("reads(g) gives the value and no part of the block"): invalid-free at the free call (as heap_free_borrowed.c)
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

void drop(void)
    CERASTES_REQUIRES(cerastes_reads(g) && (g == NULL || cerastes_live(g)))
    CERASTES_ENSURES(cerastes_reads(g) && (g == NULL || cerastes_live(g)))
{
  free(g);  // <- here
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
