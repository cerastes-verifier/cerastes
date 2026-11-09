// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 5 (retires reject_ref_to_uninit_global.c, the same declaration shape: external linkage, "= NULL"), default mode: the marker documents intent and the Viper does not change; main stores malloc's block in the marked global, writes and reads it, frees it and sets it back to NULL
// Pre-registered from record 0054 item 5 ("Marked globals"): verifies in both modes
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
