// expect: error invalid-free @ L13
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/vlibc_owns_vlibc_block.c
// Pins: 0052 item 7: the callee holds a vlibc block (token value false) and frees it with the platform's free: invalid-free (0050 item 3)
// Pre-registered from record 0052 item 7: invalid-free at the free
#include <vlibc/stdlib.h>
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns_vlibc_block(p))
{
  free(p);  // <- here
}
