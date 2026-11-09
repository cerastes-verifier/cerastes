// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 5 ("The initial value", audit B2; retires reject_owning_global.c's shape): an owning global with external linkage and no initializer (a C tentative definition, a C++ namespace-scope variable) has an unknown initial value: main sets it to NULL before the first call of a function whose contract has the deep atom; it ends holding g's block (exempt)
// Pre-registered from record 0054 item 5 and 0041 item 3: verifies in both modes
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
  g = NULL;
  init();
  init();
  return 0;
}
