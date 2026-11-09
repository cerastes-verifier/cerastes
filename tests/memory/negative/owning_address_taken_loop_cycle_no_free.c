// expect: error leak @ L18
// mode: c23, c++20
// twin: tests/memory/owning_address_taken_loop_cycle.c
// Pins: 0054 item 6 and audit round 2 (Parts C and D), probe h4: the loop allocates into the address-taken owner without freeing the block it holds
// Pre-registered from record 0054 item 6 ("Stores") and audit probe h4 as observed: leak at the store
#include <cerastes.h>
#include <stdlib.h>

int f(void)
{
  CERASTES_OWNING int *v = NULL;
  int **q = &v;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {

    v = (int *)malloc(sizeof(int));  // <- here
    if (v != NULL) {
      *v = i;
    }
    i += 1;
  }
  free(*q);
  return 0;
}

int main(void)
{
  return f();
}
