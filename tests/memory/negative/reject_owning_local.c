// expect: rejected wrong_entity_for_attribute @ L11
// mode: c23, c++20
// twin: tests/memory/owning_param.c
// Pins: 0040 item 3: owning on a local: EDG's own diagnostic (target p only; P62), re-expected by M4.5d2
#include <cerastes.h>
#include <stddef.h>

void g(void)
{
  int x = 0;
  CERASTES_OWNING int *q = &x;
  *q = 1;
}
