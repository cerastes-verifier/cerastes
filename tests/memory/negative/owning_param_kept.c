// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/owning_param.c
// Pins: 0040 item 3: keep consumes the cell (no ensures), so owning's ensures p == NULL || owns(*p) fails at the end of the function
#include <cerastes.h>
#include <stddef.h>

void keep(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

void pass_keep(CERASTES_OWNING int *p)
{
  if (p) {
    keep(p);
  }
}
