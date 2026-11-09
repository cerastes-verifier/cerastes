// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init
// Pins: 0054 item 5 (amends 0052 item 4) under --strict-init: a marked global may receive only a null pointer, the result of a ref_to_uninit call (malloc), &x of an uninit local or another marked variable: malloc's block and NULL are accepted
// Pre-registered from record 0054 item 5 ("Marked globals ... Under --strict-init"): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

CERASTES_REF_TO_UNINIT int *g = NULL;

int main(void)
{
  g = (int *)malloc(sizeof(int));
  if (g == NULL) {
    return 1;
  }
  *g = 3;
  CERASTES_CHECK(*g == 3);
  free(g);
  g = NULL;
  return 0;
}
