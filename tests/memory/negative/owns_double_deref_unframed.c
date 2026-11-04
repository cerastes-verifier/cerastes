// expect: error permission @ L9
// mode: c23, c++20
// twin: tests/memory/owns_double_deref.c
// Pins: 0045 items 3, 4: the old rejected shape: owns(**pp) with nothing owning *pp: permission at the clause
#include <cerastes.h>
#include <stddef.h>

void f(int **pp)
    CERASTES_REQUIRES(cerastes_owns(**pp))  // <- here
{
  (void)pp;
}
