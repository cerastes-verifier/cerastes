// expect: error leak @ L15
// mode: c23, c++20
// twin: tests/memory/owning_address_taken_alias_store.c
// Pins: 0054 item 6 ("Stores"): the store through the alias (guarded by pp == &v) drops the block v holds
// Pre-registered from record 0054 item 6 (S39 f1x): leak at the store through the alias
#include <cerastes.h>
#include <stdlib.h>

int alias_store(void)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  int **pp = &v;

  *pp = (int *)malloc(sizeof(int));  // <- here
  free(v);
  return 0;
}
