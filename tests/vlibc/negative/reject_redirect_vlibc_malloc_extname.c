// expect: rejected cerastes-unsupported_declaration @ L13
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/vlibc_alloc_roundtrip.c
// Pins: 0050 items 2, 8 (0047 item 2, audit B2; 0046 item 8): `#pragma redefine_extname vlibc_malloc my_vlibc_malloc` before the header gives vlibc_malloc another external name, so a call of it does not reach vlibc's contract (the pragma is not a declaration: the rejection is at the call)
// Pre-registered from record 0050 item 8: rejected unsupported_declaration at the call of vlibc_malloc, as reject_redirect_malloc_extname.c for malloc
#pragma redefine_extname vlibc_malloc my_vlibc_malloc
#include <vlibc/stdlib.h>
#include <cerastes.h>

int f(void)
{
  int *p = (int *)vlibc_malloc(sizeof(int));  // <- here
  vlibc_free(p);
  return 0;
}
