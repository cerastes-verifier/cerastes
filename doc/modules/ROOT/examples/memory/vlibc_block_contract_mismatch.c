// expect: error permission @ L20
// mode: c23
// c-only: vlibc_malloc creates no C++ objects; cerastes rejects cerastes_owns_vlibc_block in C++ (decisions 0050 item 7 and 0052 item 7)
// A block of the platform's allocator passed to a function that names vlibc's.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns_vlibc_block(p));

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 1;
  dispose(p);
  return 0;
}
// end::source[]
