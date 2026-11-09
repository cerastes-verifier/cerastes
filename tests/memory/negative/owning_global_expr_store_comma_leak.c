// expect: error leak @ L19
// mode: c23, c++20
// twin: tests/memory/owning_global_expr_store_comma.c
// Pins: 0054 item 5 and audit round 2 (Parts C and D), probe g (comma): the comma store overwrites the block g already holds
// Pre-registered from derived from the record (not observed): leak at the line of the comma store (the early return also leaves g's block exempt: only the overwrite leaks)
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING static int *g = NULL;

int main(void)
{
  g = (int *)malloc(sizeof(int));
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return 1;
  }
  *p = 1;
  int x = (g = p, 0);  // <- here
  return x + (g == NULL);
}
