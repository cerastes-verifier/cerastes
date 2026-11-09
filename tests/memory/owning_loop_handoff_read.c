// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 3 (e) (question 181; S39 d2f): *w is read in every iteration, so also after v = w; handed the block to the owner: the owner's carrier gains the flag clause for a block that a place carrier (w) also names (guarded by the entry flag snapshot and amount), so the read is initialized; abort() is the vacuous branch the compilers cannot see is dead
// Pre-registered from record 0054 item 3 (e) and S39 d2f: verifies in both modes (no cut needed)
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
  CERASTES_INVARIANT(0 <= i && i <= 4 && (i < 3 || v != NULL))
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    if (i == 2 && v == NULL) {
      v = w;
    }
    if (*w != 0) {
      abort();
    }
    i += 1;
  }
  free(v);
  return 0;
}
