// expect: error permission @ L27
// expect: error leak @ L30
// mode: c23, c++20
// twin: tests/memory/owning_loop_alias.c
// Pins: 0052 item 3 Loops, audit 3b (m3_alias_becomes_owned.c), a documented false alarm lifted by M4.5d2b2: an owner takes the block of a non-owning pointer w inside the loop (v = w, v NULL at entry): the loop already carries the block through w: permission at the back edge
// Pre-registered from audit round 3b (build/m4.5d2b/audit3b.md): permission at the back edge and, as a consequence of the same fault, a leak at the function's end (both kept: a documented false alarm; the lines are the first run's)
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
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (i == 2 && v == NULL) {
      v = w;
    }
    *w = i;
    i += 1;
  }  // <- here
  free(v);
  return 0;
}  // <- here
