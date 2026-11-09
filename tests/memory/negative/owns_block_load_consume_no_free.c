// expect: error leak @ L13
// mode: c23, c++20
// twin: tests/memory/owns_block_load_consume.c
// Pins: 0054 item 2 ("consumed"): a function that takes a block through *pp and neither frees it nor hands it back (no ensures site) leaks it at its closing brace
// Pre-registered from record 0054 item 2 (S39 ap1z: consume forgets free, no exemption without an ensures site): leak at the closing brace
#include <cerastes.h>
#include <stdlib.h>

void dispose(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns_block(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp))
{
}  // <- here

int make_and_dispose(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 5;
  dispose(&p);
  p = NULL;
  return 0;
}
