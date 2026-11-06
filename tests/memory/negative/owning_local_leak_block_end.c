// expect: error leak @ L18
// mode: c23, c++20
// twin: tests/memory/owning_local_free.c
// Pins: 0052 item 3: the owning local's block still held when its block ends: leak at the closing brace of the block (the scope exit), not at the function's end
// Pre-registered from record 0052 item 3: leak at the block's closing brace
#include <cerastes.h>
#include <stdlib.h>

int block_scope(int c)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  {
    CERASTES_OWNING int *q = (int *)malloc(sizeof(int));
    if (q == NULL) {
      return -1;
    }
    *q = c;
  }  // <- here
  return 0;
}
