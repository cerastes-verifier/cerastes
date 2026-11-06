// expect: error permission @ L14
// mode: c23, c++20
// twin: tests/memory/owning_beside_block.c
// Pins: 0052 item 6c: the callee frees the block the merged site promises back: permission at the closing brace
// Pre-registered from record 0052 item 6c: permission at the closing brace
#include <cerastes.h>
#include <stdlib.h>

void touch(CERASTES_OWNING int *p)
    CERASTES_REQUIRES(cerastes_owns_block(p))
    CERASTES_ENSURES(cerastes_owns_block(p))
{
  free(p);
}  // <- here
