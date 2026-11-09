// expect: error permission @ L25
// mode: c23, c++20
// twin: tests/memory/owns_block_load_guard.c
// Pins: 0054 item 2: with *pp non-null the guard demands the block, and p holds the address of a local, no block: the call needs a permission the caller does not hold
// Pre-registered from record 0054 item 2 (calls: 0047 item 4's rows on the pointer the callee's place designates): permission at the call (as heap_consume_borrowed.c)
#include <cerastes.h>
#include <stdlib.h>

void renew(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && (*pp == NULL || cerastes_owns_block(*pp)))
    CERASTES_ENSURES(cerastes_owns(*pp) && (*pp == NULL || cerastes_owns_block(*pp)))
{
  free(*pp);
  *pp = (int *)malloc(sizeof(int));
  if (*pp != NULL) {
    **pp = 1;
  }
}

int use_renew(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 0;
  int *p = &x;
  renew(&p);  // <- here
  return 0;
}
