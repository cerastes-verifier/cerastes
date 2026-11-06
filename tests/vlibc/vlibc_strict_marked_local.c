// expect: verified
// expect: native-ok
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// verify-options: --strict-init
// Pins: 0052 item 4 (0050 item 2), --strict-init: a marked local receiving vlibc_malloc's result, freed with vlibc_free (exempt like free)
// Pre-registered from record 0052 item 4: verifies under --strict-init
#include <vlibc/stdlib.h>
#include <cerastes.h>

int f(void)
    CERASTES_ENSURES(r, r == 3 || r == -1)
{
  CERASTES_REF_TO_UNINIT int *p = (int *)vlibc_malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 3;
  int v = *p;
  vlibc_free(p);
  return v;
}
