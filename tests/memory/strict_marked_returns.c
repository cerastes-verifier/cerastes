// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init
// Pins: 0052 item 4 (question 154, differs from P4416 p6, which counts null as initialized and would reject every allocation wrapper's return NULL), --strict-init: a ref_to_uninit function returns a marked local, a null pointer constant or a call of a ref_to_uninit function; its caller marks the local that receives the result
// Pre-registered from record 0052 item 4: verifies under --strict-init in both modes
#include <cerastes.h>
#include <stdlib.h>

CERASTES_REF_TO_UNINIT int *raw(void)
    CERASTES_ENSURES(r, r == NULL || cerastes_owns_block(r))
{
  CERASTES_REF_TO_UNINIT int *p = (int *)malloc(sizeof(int));
  return p;
}

CERASTES_REF_TO_UNINIT int *raw_null(void)
    CERASTES_ENSURES(r, r == NULL || cerastes_owns_block(r))
{
  return NULL;
}

CERASTES_REF_TO_UNINIT int *raw_call(void)
    CERASTES_ENSURES(r, r == NULL || cerastes_owns_block(r))
{
  return raw();
}

CERASTES_REF_TO_UNINIT int *raw_direct(void)
    CERASTES_ENSURES(r, r == NULL || cerastes_owns_block(r))
{
  return (int *)malloc(sizeof(int));
}

int use_raw(void)
    CERASTES_ENSURES(r, r == 4 || r == -1)
{
  CERASTES_REF_TO_UNINIT int *p = raw_call();
  if (p == NULL) {
    return -1;
  }
  *p = 4;
  int v = *p;
  free(p);
  return v;
}
