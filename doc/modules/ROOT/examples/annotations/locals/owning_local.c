// expect: verified
// expect: native-ok
// mode: c23, c++20
// An owning local in both positions: before the declaration (it covers every declarator) and after the declarator's name (it covers that one).
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int leading(void)
    CERASTES_ENSURES(r, r == 2 || r == -1)
{
  CERASTES_OWNING int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 2;
  int v = *p;
  free(p);
  return v;
}

int trailing(void)
    CERASTES_ENSURES(r, r == 5 || r == -1)
{
  int x = 3;
  int *p CERASTES_OWNING = (int *)malloc(sizeof(int)), *q = &x;
  if (p == NULL) {
    return -1;
  }
  *p = 2;
  int v = *p + *q;
  free(p);
  return v;
}
// end::source[]
