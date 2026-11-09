// expect: error leak @ L23
// mode: c23, c++20
// twin: tests/memory/owning_loop_alias_becomes_owned.c
// Pins: 0054 item 3: the owner that took w's block lets go of it again: v = NULL drops a block that only v owns (w is a plain alias)
// Pre-registered from record 0054 item 3 (S39 d2c): leak at v = NULL (the reassignment row ends the path)
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
      v = NULL;  // <- here
    }
    *w = i;
    i += 1;
  }
  free(v);
  return 0;
}
