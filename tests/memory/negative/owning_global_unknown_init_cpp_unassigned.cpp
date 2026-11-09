// expect: error permission @ L23
// expect: error permission @ L23
// twin: tests/memory/owning_global_unknown_init_cpp.cpp
// Pins: 0054 item 5: the same without the assignment: permission at the call. Owner question 187: the same-line duplicate (two identical permission errors at the call, question 112 precedent) is registered twice. Pre-registered from record 0054 item 5 (audit B2): permission at the call
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING int *g = nullptr;

void init()
    CERASTES_REQUIRES(cerastes::owns(g))
    CERASTES_ENSURES(cerastes::owns(g))
{
  free(g);
  g = static_cast<int *>(malloc(sizeof(int)));
  if (g != nullptr) {
    *g = 1;
  }
}

int main()
{
  init();  // <- here
  return 0;
}
