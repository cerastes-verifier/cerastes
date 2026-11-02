// expect: error uninitialized @ L18
// mode: c23, c++20
// twin: tests/memory/init_strict_shapes_off.c
// Pins: 0042 item 8: without the option the same unmarked shapes are still checked path by path (0019)
#include <cerastes.h>
#include <stddef.h>
void init(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 0);

int c(void)
{
  int x;
  int *q = &x;
  if (q == NULL) {
    return 0;
  }
  return x;  // <- here
}
