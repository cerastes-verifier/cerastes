// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 3 (d) (S39 w2b): an owning local freed through an alias before a loop and left non-null (held_a is false), with a call in the loop: a is a full carrier guarded by held_a, so the loop's entry does not newly fail and a's exit row finds nothing held
// Pre-registered from record 0054 item 3 (d) and S39 w2b: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

void touch(void) {}

int main(void)
{
  CERASTES_OWNING int *a = (int *)malloc(sizeof(int));
  if (a == NULL) {
    return 1;
  }
  *a = 0;
  int *al = a;
  free(al);
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    touch();
    i += 1;
  }
  return 0;
}
