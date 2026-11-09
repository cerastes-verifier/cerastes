// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 3 and owner question 180 (retires owning_loop_alias_becomes_owned_false_alarm.c: the pin's program with i < 3 || v != NULL added to its invariant): an owner takes the block of a non-owning pointer w inside the loop (v = w, v NULL at entry); if the loop could end with v NULL, w's block would leak, so an owner assigned on some iterations needs such an invariant
// Pre-registered from record 0054 item 3 and S39 d2b (s39_d_alias_becomes_owned.vpr): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int main(void)
{
  int *w = (int *)malloc(sizeof(int));
  if (w == NULL) {
    return 1;
  }
  *w = 0;
  CERASTES_OWNING int *v = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && (i < 3 || v != NULL))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (i == 2 && v == NULL) {
      v = w;
    }
    *w = i;
    i += 1;
  }
  free(v);
  return 0;
}
