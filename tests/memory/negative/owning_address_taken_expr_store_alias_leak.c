// expect: error leak @ L13
// mode: c23, c++20
// twin: tests/memory/owning_address_taken_expr_store_alias.c
// Pins: 0054 item 6 and audit round 2 (Parts C and D), probe g4: the same store drops the block v holds
// Pre-registered from audit probe g4 as observed: leak at the line of the expression store through the alias
#include <cerastes.h>
#include <stdlib.h>

int f(void)
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  int **q = &v;
  int r = (*q = NULL) == NULL;  // <- here
  return r - 1;
}

int main(void)
{
  return f();
}
