// expect: verified
// expect: native-ok
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// Pins: 0054 item 2 (0052 item 7): cerastes_owns_vlibc_block(*pp) through a load, given by a callee that allocates with vlibc_malloc; the token's value is false, and the caller frees with vlibc_free
// Pre-registered from record 0054 item 2 (forms: "cerastes_owns_vlibc_block(E)"): verifies
#include <vlibc/stdlib.h>
#include <cerastes.h>

void make(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns_vlibc_block(*pp) && **pp == 5)
{
  *pp = (int *)vlibc_malloc(sizeof(int));
  if (*pp == NULL) {
    abort();
  }
  **pp = 5;
}

int use_make(void)
    CERASTES_ENSURES(r, r == 5)
{
  int *p = NULL;
  make(&p);
  int v = *p;
  vlibc_free(p);
  return v;
}
