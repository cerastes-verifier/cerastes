// expect: error lifetime @ L25
// expect: error invalid-free @ L26
// mode: c23, c++20
// twin: tests/memory/owning_global_unknown_init.c
// Pins: 0054 item 5 ("The initial value"): program_start gives no block, so an unknown g that main finds non-null holds nothing main may free: "if (g) free(g);" at main's start is invalid-free, not dead code
// Pre-registered from record 0054 item 5 (audit B2): invalid-free at the free call, AND (BEST READING, not in the record) lifetime at the test of the unknown pointer: today's encoder reports lifetime for every read of a pointer cell whose liveness is unknown (k3 probe: "if (g) free(g);" on a non-owning unknown global gives both); see the report
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
  if (g) {  // <- here
    free(g);  // <- here
  }
  g = NULL;
  init();
  return 0;
}
