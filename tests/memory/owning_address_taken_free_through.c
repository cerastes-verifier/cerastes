// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 6 (retires reject_owning_address_taken.c, the same program): an owning local whose address is taken (int **pp = &p): the block is freed through the alias, held_p is false after the free, and p's exit row finds nothing held
// Pre-registered from record 0054 item 6: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int free_through(void)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_OWNING int *p = (int *)malloc(sizeof(int));
  int **pp = &p;
  free(*pp);
  return 0;
}
