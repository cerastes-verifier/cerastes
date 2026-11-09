// expect: error leak @ L18
// mode: c23, c++20
// twin: tests/memory/owning_global_three.c
// Pins: 0054 item 5 ("Stores") and audit round 2 (Parts C and D), probe i1: one of the three owning globals is overwritten while it holds a block
// Pre-registered from record 0054 item 5: leak at the second assignment to b
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING static int *a = NULL;
CERASTES_OWNING static int *b = NULL;
CERASTES_OWNING static int *c = NULL;

int main(void)
{
  a = (int *)malloc(sizeof(int));
  b = (int *)malloc(sizeof(int));
  c = (int *)malloc(sizeof(int));
  b = (int *)malloc(sizeof(int));  // <- here
  return a == b || b == c;
}
