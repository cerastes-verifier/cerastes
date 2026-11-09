// expect: error leak @ L25
// mode: c23, c++20
// twin: tests/memory/owning_address_taken_guarded_site.c
// Pins: 0054 item 6 and audit round 2 (Parts C and D), probe c4: on the path where maybe() returned the block (c != 0) the owner is never freed
// Pre-registered from record 0054 item 6: leak at the return (v's exit row)
#include <cerastes.h>
#include <stdlib.h>

void maybe(int **pp, int c)
    CERASTES_REQUIRES(cerastes_owns(*pp) && (*pp == NULL || cerastes_owns_block(*pp)))
    CERASTES_ENSURES(cerastes_owns(*pp) && (c == 0 || (*pp == NULL || cerastes_owns_block(*pp))))
{
  if (c == 0) {
    free(*pp);
  }
}

int use(int c)
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  if (v != NULL) {
    *v = 0;
  }
  maybe(&v, c);
  return 0;  // <- here
}
