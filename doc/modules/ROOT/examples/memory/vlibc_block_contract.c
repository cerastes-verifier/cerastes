// expect: verified
// expect: native-ok
// mode: c23
// c-only: vlibc_malloc creates no C++ objects; cerastes rejects cerastes_owns_vlibc_block in C++ (decisions 0050 item 7 and 0052 item 7)
// A function borrows a block of vlibc's allocator, another consumes one with vlibc_free; both name it with cerastes_owns_vlibc_block.
// tag::source[]
#include <cerastes.h>
#include <vlibc/stdlib.h>

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
// end::source[]
