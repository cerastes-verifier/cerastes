// expect: error permission @ L14
// mode: c23, c++20
// twin: tests/memory/owning_global_expr_store.c
// Pins: 0054 item 5 (the value rule inside an expression) and audit round 2 (Parts C and D), probe g5: the expression store gives the owning global the address of a local
// Pre-registered from audit probe g5 as observed: permission at the line of the expression store
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING static int *g = NULL;

int main(void)
{
  int x = 0;
  int r = (g = &x) == NULL;  // <- here
  return r + (g == NULL);
}
