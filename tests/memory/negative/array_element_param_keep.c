// expect: error lifetime @ L15
// mode: c23, c++20
// twin: tests/memory/array_element_param.c
// Pins: 0059 item 6: a callee kept one element, so the scope exit finds the array not whole: lifetime at the closing brace
// Pre-registered from record 0059 item 6
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

void keep(void)
{
  int a[3] = {1, 2, 3};
  dispose(&a[1]);
}
