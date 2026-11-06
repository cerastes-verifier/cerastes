// expect: error permission @ L14
// mode: c23, c++20
// twin: tests/memory/block_site_given_formal.c
// Pins: 0052 item 2: the callee frees the block it was given and returns owns_block(b) of a pointer it holds nothing of: a block site has no amount, so the callee must hold the whole block: permission at the closing brace
// Pre-registered from record 0052 item 2: permission at the closing brace (as formal_pointee_not_held.c)
#include <cerastes.h>
#include <stdlib.h>

void rebadge(int *a, int *b)
    CERASTES_REQUIRES(cerastes_owns_block(a))
    CERASTES_ENSURES(cerastes_owns_block(b))
{
  free(a);
}  // <- here
