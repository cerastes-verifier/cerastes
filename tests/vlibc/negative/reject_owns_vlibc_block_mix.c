// expect: rejected cerastes-unsupported_annotation @ L13
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/vlibc_owns_vlibc_block.c
// Pins: 0052 item 7 (audit S4, e_vlibc_mix.c): a contract that requires a vlibc block and ensures a platform block on one root (a borrow that changes the allocator) would let a vlibc block be freed with free: rejected
// Pre-registered from audit round 2 (build/m4.5d2b/audit2.md) S4: rejected unsupported_annotation at the clause line (a guess; the first run's line wins)
#include <cerastes.h>
#include <stdlib.h>
#include <vlibc/stdlib.h>

void retag(int *p)
    CERASTES_REQUIRES(cerastes_owns_vlibc_block(p))
    CERASTES_ENSURES(cerastes_owns_block(p));  // <- here

int main(void)
{
  int *p = (int *)vlibc_malloc(sizeof(int));
  if (p == NULL) {
    return 1;
  }
  *p = 1;
  retag(p);
  free(p);
  return 0;
}
