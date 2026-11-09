// expect: error leak @ L14
// mode: c23, c++20
// twin: tests/memory/owning_address_taken_expr_store.c
// Pins: 0054 item 6 and audit round 2 (Parts C and D), probe g3: the store inside the condition overwrites a block v holds
// Pre-registered from audit probe g3 as observed: leak at the line of the expression store
#include <cerastes.h>
#include <stdlib.h>

int f(void)
{
  CERASTES_OWNING int *v = NULL;
  int **q = &v;
  v = (int *)malloc(sizeof(int));
  if ((v = (int *)malloc(sizeof(int))) == NULL) {  // <- here
    return 1;
  }
  free(*q);
  return 0;
}

int main(void)
{
  return f();
}
