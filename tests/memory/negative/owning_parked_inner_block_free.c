// expect: error permission @ L19
// mode: c23, c++20
// twin: tests/memory/owning_parked_inner_block.c
// Pins: 0052 item 3 (audit S3): the owning local of the inner block frees the borrowed block, and the function writes it afterwards: permission at that write
// Pre-registered from audit round 2 (build/m4.5d2b/audit2.md) S3: permission at the write
#include <cerastes.h>
#include <stdlib.h>

void touch(int *p)
    CERASTES_REQUIRES(cerastes_owns_block(p))
    CERASTES_ENSURES(cerastes_owns_block(p))
{
  int k = 0;
  {
    CERASTES_OWNING int *v = p;
    free(v);
  }
  k = 1;
  *p = k;  // <- here
}
