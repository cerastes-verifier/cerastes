// expect: error uninitialized @ L23
// mode: c23, c++20
// twin: tests/memory/owning_loop_handoff_read.c
// Pins: 0054 item 3 (e) (S39 d2g): the same loop whose block has no value at entry (the write is gone): the flag clause is only given for a flag that was set at entry, so the read stays uninitialized (the first iteration reads it before anything wrote it)
// Pre-registered from record 0054 item 3 (e): uninitialized at the read (one line: every iteration's read is that line)
#include <cerastes.h>
#include <stdlib.h>

int main(void)
{
  int *w = (int *)malloc(sizeof(int));
  if (w == NULL) {
    return 1;
  }
  CERASTES_OWNING int *v = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4 && (i < 3 || v != NULL))
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    if (i == 2 && v == NULL) {
      v = w;
    }
    if (*w != 0) {  // <- here
      abort();
    }
    i += 1;
  }
  free(v);
  return 0;
}
