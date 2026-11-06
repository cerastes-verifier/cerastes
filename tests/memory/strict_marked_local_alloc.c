// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init
// Pins: 0052 item 4 (P4222R3, P4416; amends 0042 item 8 and 0047 item 8), --strict-init: a local that receives malloc's result by initialization or by assignment is marked ref_to_uninit; calloc's result (initialized) needs no marker; free's argument is exempt
// Pre-registered from record 0052 item 4: verifies under --strict-init in both modes
#include <cerastes.h>
#include <stdlib.h>

int via_init(void)
    CERASTES_ENSURES(r, r == 3 || r == -1)
{
  CERASTES_REF_TO_UNINIT int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 3;
  int v = *p;
  free(p);
  return v;
}

int via_assign(void)
    CERASTES_ENSURES(r, r == 4 || r == -1)
{
  CERASTES_REF_TO_UNINIT int *p = NULL;
  p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 4;
  int v = *p;
  free(p);
  return v;
}

int via_calloc(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *q = (int *)calloc(1, sizeof(int));
  if (q == NULL) {
    return -1;
  }
  int v = *q;
  free(q);
  return v;
}
