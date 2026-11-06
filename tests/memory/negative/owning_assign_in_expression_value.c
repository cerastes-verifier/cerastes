// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/owning_assign_in_expression.c
// Pins: 0052 item 3 (audit S2, d_assign_expr.c): (v = &g) == NULL: the value rule applies to an assignment inside an expression: &g is not a block the function holds whole: permission at the assignment
// Pre-registered from audit round 2 (build/m4.5d2b/audit2.md) S2: permission at the assignment
#include <cerastes.h>
#include <stdlib.h>

int g;

int f(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  CERASTES_OWNING int *v = NULL;
  if ((v = &g) == NULL) {  // <- here
    return 1;
  }
  *v = 2;
  return 0;
}
