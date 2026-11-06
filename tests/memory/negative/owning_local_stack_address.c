// expect: error permission @ L11
// mode: c23, c++20
// twin: tests/memory/owning_local_free.c
// Pins: 0052 item 3 (the value rule): an owning local initialized with the address of a local variable: that pointer is not a block the function holds whole (no heap token), so permission at the initialization
// Pre-registered from record 0052 item 3: permission at the initialization
#include <cerastes.h>

void g(void)
{
  int x = 0;
  CERASTES_OWNING int *q = &x;  // <- here
  *q = 1;
}
