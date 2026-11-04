// expect: error permission @ L9
// mode: c23, c++20
// twin: tests/memory/read_double_deref.c
// Pins: 0045 items 3, 4: the old rejected shape: **pp read with no atom on it: permission at the clause
#include <cerastes.h>
#include <stddef.h>

void f(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && **pp > 0)  // <- here
{
  (void)pp;
}
