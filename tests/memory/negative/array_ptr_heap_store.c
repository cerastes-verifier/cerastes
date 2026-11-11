// expect: error permission @ L17
// mode: c23, c++20
// twin: tests/memory/owning_global_expr_store.c
// Pins: 0059 item 10 (audit round 1, item 10): &p[1] of a one-object heap block is not the block's start: storing it into an owning global is permission at the store (the owning-assignment rows require offset_of == 0); it is never a leak
// Pre-registered from record 0059 item 10 (Part D), before the code
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING static int *g = NULL;

int main(void)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return 1;
  }
  g = &p[1];
  *g = 1;
  return 0;
}
