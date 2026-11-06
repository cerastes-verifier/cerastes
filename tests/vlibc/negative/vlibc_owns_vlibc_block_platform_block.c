// expect: error permission @ L24
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/vlibc_owns_vlibc_block.c
// Pins: 0052 item 7: the caller passes a block of the platform's allocator where cerastes_owns_vlibc_block is required: the token's value is wrong, permission at the call
// Pre-registered from record 0052 item 7: permission at the call
#include <vlibc/stdlib.h>
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns_vlibc_block(p))
{
  vlibc_free(p);
}

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 1;
  dispose(p);  // <- here
  return 0;
}
