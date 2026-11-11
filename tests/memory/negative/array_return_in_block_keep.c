// expect: error lifetime @ L17
// mode: c23, c++20
// twin: tests/memory/array_return_in_block.c
// Pins: 0059 item 6: the callee kept a[0]; the return leaves the block, and the scope exit at the return reports lifetime
// Pre-registered from record 0059 item 6
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int early(int c)
    CERASTES_ENSURES(r, r == (c != 0 ? 1 : 0))
{
  if (c != 0) {
    int a[2] = {1, 2};
    dispose(&a[0]);
    return 1;
  }
  return 0;
}
