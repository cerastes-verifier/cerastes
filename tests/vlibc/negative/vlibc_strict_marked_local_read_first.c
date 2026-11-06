// expect: error uninitialized @ L18
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// verify-options: --strict-init
// twin: tests/vlibc/vlibc_strict_marked_local.c
// Pins: 0052 item 4: the marked local that received vlibc_malloc's result is read before it is written: uninitialized
// Pre-registered from record 0052 item 4: uninitialized at the read
#include <vlibc/stdlib.h>
#include <cerastes.h>

int f(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_REF_TO_UNINIT int *p = (int *)vlibc_malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int v = *p;  // <- here
  *p = v;
  vlibc_free(p);
  return 0;
}
