// expect: error leak @ L14
// mode: c23, c++20
// twin: tests/memory/owning_global_expr_store.c
// Pins: 0054 item 5 ("Stores ... including one inside an expression") and audit round 2 (Parts C and D), probe g1: the store inside the condition overwrites a block g holds
// Pre-registered from audit probe g1 as observed: leak at the line of the expression store
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING static int *g = NULL;

int main(void)
{
  g = (int *)malloc(sizeof(int));
  if ((g = (int *)malloc(sizeof(int))) == NULL) {  // <- here
    return 1;
  }
  return g == NULL;
}
