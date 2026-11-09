// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 3 (retires owning_loop_becomes_shared_false_alarm.c, the SAME program): a second owner takes the first owner's block inside the loop (b = a, b NULL at entry): the block becomes shared there and the current-state carriers (guard, exclusivity clause, value set b == eb || b == a) hand it to the earlier carrier; a is freed after the loop
// Pre-registered from record 0054 item 3 and S39 d1b (s39_d_becomes_shared.vpr): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int main(void)
{
  CERASTES_OWNING int *a = (int *)malloc(sizeof(int));
  if (a == NULL) {
    return 1;
  }
  *a = 0;
  CERASTES_OWNING int *b = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (i == 2 && b == NULL) {
      b = a;
    }
    i += 1;
  }
  free(a);
  return 0;
}
