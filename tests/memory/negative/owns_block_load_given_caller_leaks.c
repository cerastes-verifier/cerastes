// expect: error leak @ L27
// mode: c23, c++20
// twin: tests/memory/owns_block_load_given.c
// Pins: 0054 item 2: the block a callee gave through *pp belongs to the caller; the caller that never frees it leaks it at its closing brace
// Pre-registered from record 0054 item 2 ("given", S39 a1y): leak at the closing brace (one row per function)
#include <cerastes.h>
#include <stdlib.h>

void make(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns_block(*pp) && **pp == 5)
{
  *pp = (int *)malloc(sizeof(int));
  if (*pp == NULL) {
    abort();
  }
  **pp = 5;
}

int use_make(void)
    CERASTES_ENSURES(r, r == 5)
{
  int *p = NULL;
  make(&p);
  int v = *p;
  return v;
}  // <- here
