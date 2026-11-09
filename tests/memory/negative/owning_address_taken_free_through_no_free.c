// expect: error leak @ L17
// mode: c23, c++20
// twin: tests/memory/owning_address_taken_free_through.c
// Pins: 0054 item 6: the owning address-taken local still holds its block at its exit: leak at the return (0052 item 3's exit row, the value read from the local's cell)
// Pre-registered from record 0054 item 6 (S39 f1y): leak at the return statement
#include <cerastes.h>
#include <stdlib.h>

int free_through(void)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_OWNING int *p = (int *)malloc(sizeof(int));
  int **pp = &p;
  if (*pp != NULL) {
    **pp = 1;
  }
  return 0;  // <- here
}
