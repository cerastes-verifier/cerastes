// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 11 (question 109): a guard that reads the heap (*pp == NULL || owns(**pp)): with *pp null the caller passes without owning, with *pp non-null it must own
#include <cerastes.h>
#include <stddef.h>

void h(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && (*pp == NULL || cerastes_owns(**pp)))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && (*pp == NULL || cerastes_owns(**pp)))
    CERASTES_ENSURES(*pp == NULL || **pp == 1)
{
  if (*pp != NULL) {
    **pp = 1;
  }
}

int use_null(void)
    CERASTES_ENSURES(r, r == 0)
{
  int *p = NULL;
  h(&p);
  return 0;
}

int use_nonnull(void)
    CERASTES_ENSURES(r, r == 1)
{
  int x = 0;
  int *p = &x;
  h(&p);
  CERASTES_CHECK(x == 1);
  return x;
}
