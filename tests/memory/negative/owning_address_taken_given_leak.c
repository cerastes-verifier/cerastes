// expect: error leak @ L26
// mode: c23, c++20
// twin: tests/memory/owning_address_taken_given.c
// Pins: 0054 item 6: the owner received a block through make(&v) and never frees it
// Pre-registered from record 0054 item 6 (S39 f1y): leak at the return statement (v's exit row)
#include <cerastes.h>
#include <stdlib.h>

void make(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns_block(*pp))
{
  *pp = (int *)malloc(sizeof(int));
  if (*pp == NULL) {
    abort();
  }
  **pp = 5;
}

int use_make(void)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_OWNING int *v = NULL;
  make(&v);

  return 0;  // <- here
}
