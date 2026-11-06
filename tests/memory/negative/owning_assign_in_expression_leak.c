// expect: error leak @ L16
// mode: c23, c++20
// twin: tests/memory/owning_assign_in_expression.c
// Pins: 0052 item 3 (audit S2): the same idiom without the free: the owner still holds its block at the return: leak
// Pre-registered from audit round 2 (build/m4.5d2b/audit2.md) S2: leak at the return
#include <cerastes.h>
#include <stdlib.h>

int alloc_in_condition(void)
{
  CERASTES_OWNING int *v = NULL;
  if ((v = (int *)malloc(sizeof(*v))) == NULL) {
    return 1;
  }
  *v = 1;
  return 0;  // <- here
}
