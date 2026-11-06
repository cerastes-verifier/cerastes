// expect: verified
// expect: native-ok
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// Pins: 0052 item 7 (question 153): cerastes_owns_vlibc_block(p) in contracts: a callee that borrows a vlibc block and one that consumes it with vlibc_free; the caller allocates with vlibc_malloc
// Pre-registered from record 0052 item 7: verifies
#include <vlibc/stdlib.h>
#include <cerastes.h>

void touch(int *p)
    CERASTES_REQUIRES(cerastes_owns_vlibc_block(p))
    CERASTES_ENSURES(cerastes_owns_vlibc_block(p))
{
  *p = 2;
}

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns_vlibc_block(p))
{
  vlibc_free(p);
}

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *p = (int *)vlibc_malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 1;
  touch(p);
  dispose(p);
  return 0;
}
