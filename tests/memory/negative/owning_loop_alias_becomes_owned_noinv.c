// expect: error leak @ L29
// mode: c23, c++20
// twin: tests/memory/owning_loop_alias_becomes_owned.c
// Pins: 0054 item 3 and owner question 180: the program without the i < 3 || v != NULL conjunct: the loop need not hand the block to v (it could end with v NULL), so w's block leaks at the end of main; the loop needs the invariant (S39 d2a)
// Pre-registered from record 0054 item 3 and S39 d2a: ONLY a leak at the closing brace of main (the end label); the pin's permission at the back edge is gone
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
  }
  free(v);
  return 0;
}  // <- here
