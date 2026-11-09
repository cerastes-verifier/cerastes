// expect: error permission @ L24
// expect: error permission @ L24
// mode: c23, c++20
// twin: tests/memory/owning_global_unknown_init.c
// Pins: 0054 item 5 ("The initial value"): without the assignment main does not know g is NULL (another unit may have set it), so it cannot give init() the deep atom: "calls init, which needs owning global g to hold its block or be null". Owner question 187: the same-line duplicate (two identical permission errors at the call, question 112 precedent) is registered twice. Pre-registered from record 0054 item 5 (audit B2): permission at the first call (the row ends the path)
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING int *g;

void init(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  free(g);
  g = (int *)malloc(sizeof(int));
  if (g != NULL) {
    *g = 1;
  }
}

int main(void)
{
  init();  // <- here
  init();
  return 0;
}
