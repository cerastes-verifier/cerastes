// expect: error leak @ L25
// mode: c23, c++20
// twin: tests/memory/owning_local_consume.c
// Pins: 0052 item 3: a callee that only borrows the cell (owns in requires and ensures) does not take the block, so the owning local still holds it at the return
// Pre-registered from record 0052 item 3: leak at the return
#include <cerastes.h>
#include <stdlib.h>

void touch(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  *p = 0;
}

int borrow_only(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_OWNING int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 1;
  touch(p);
  return 0;  // <- here
}
