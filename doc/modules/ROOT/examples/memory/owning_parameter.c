// expect: verified
// expect: native-ok
// mode: c23, c++20
// An owning parameter: the pointer may be null; the other clauses read *p where it is not.
// tag::source[]
#include <cerastes.h>
#include <stddef.h>

void bump(CERASTES_OWNING int *p)
    CERASTES_REQUIRES(p == NULL || *p < 100)
    CERASTES_ENSURES(p == NULL || *p == cerastes_old(*p) + 1)
{
  if (p != NULL) {
    *p += 1;
  }
}

int use_bump(void)
    CERASTES_ENSURES(r, r == 6)
{
  int x = 5;
  bump(&x);
  bump(NULL);
  return x;
}
// end::source[]
