// expect: rejected cerastes-unsupported_annotation @ L13
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// verify-options: --strict-init
// twin: tests/vlibc/vlibc_alloc_roundtrip.c
// Pins: 0050 item 2 (0047 item 8; audit round 1 S4): under --strict-init a local initialized from a ref_to_uninit result (vlibc_malloc's, like malloc's) must itself be marked ref_to_uninit (target v: P4222R3 and P4416R0, M4.5d2b)
// Pre-registered from record 0050 item 2: rejected unsupported_annotation at the call, as reject_strict_init_malloc_local.c for malloc
#include <vlibc/stdlib.h>
#include <cerastes.h>

int f(void)
{
  int *p = (int *)vlibc_malloc(sizeof(int));  // <- here
  vlibc_free(p);
  return 0;
}
