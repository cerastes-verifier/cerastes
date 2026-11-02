// expect: error uninitialized @ L17
// mode: c23, c++20
// twin: tests/memory/init_must_init.c
// Pins: 0042 item 7: only the path that calls init gives x a value; the other path reads it uninitialized (0019, path-precise)
#include <cerastes.h>

void init(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 0);

int use_init(int c)
{
  int x;
  if (c != 0) {
    init(&x);
  }
  return x;  // <- here
}
