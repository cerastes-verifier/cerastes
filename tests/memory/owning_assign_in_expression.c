// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 3 (audit S2, d_assign_expr.c): an owning local assigned inside an expression goes through the value row and the reassignment row like a statement: the idiom if ((v = malloc(...)) == NULL) { return 1; } with the owner freed, and the same assignment in an initializer's expression
// Pre-registered from audit round 2 (build/m4.5d2b/audit2.md) S2: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int alloc_in_condition(void)
{
  CERASTES_OWNING int *v = NULL;
  if ((v = (int *)malloc(sizeof(*v))) == NULL) {
    return 1;
  }
  *v = 1;
  free(v);
  return 0;
}

int alloc_in_initializer(void)
{
  {
    CERASTES_OWNING int *v = NULL;
    int x = ((v = (int *)malloc(sizeof(int))) == NULL);
    if (x) {
      return 1;
    }
    *v = 1;
    free(v);
  }
  return 0;
}
