// expect: verified
// expect: native-ok
// Pins: 0054 item 5 and 0041 item 3: C++ external linkage makes the value of "int *g = nullptr;" unknown at main's start (a dynamic initializer elsewhere may write it), so main sets it before calling init (the C++ spelling cerastes::owns(g))
// Pre-registered from record 0054 item 5 ("The initial value", question 84): verifies
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
  g = nullptr;
  init();
  return 0;
}
