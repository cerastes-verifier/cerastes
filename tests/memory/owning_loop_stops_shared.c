// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 3 (retires owning_loop_stops_shared_false_alarm.c, the SAME program): two owners share a block at entry and one lets go inside the loop (a = NULL): the block moves to the remaining carrier (b, a full carrier guarded by the current state) and is freed after the loop
// Pre-registered from record 0054 item 3 and S39 d3a (s39_d_stops_shared.vpr): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int main(void)
{
  CERASTES_OWNING int *a = (int *)malloc(sizeof(int));
  if (a == NULL) {
    return 1;
  }
  *a = 0;
  CERASTES_OWNING int *b = a;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (a != NULL) {
      a = NULL;
    }
    i += 1;
  }
  free(b);
  return 0;
}
