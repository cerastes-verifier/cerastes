// expect: error lifetime @ L20
// mode: c23, c++20
// twin: tests/memory/array_struct_address.c
// Pins: 0064 items 5 and 6: a callee that takes the element's cells and keeps them leaves the array not whole: lifetime at the closing brace, the first marked scope-exit row (S48 (a))
// Pre-registered from record 0064 items 5 and 6 (Part B), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

void dispose(struct P *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

void keep(void)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  dispose(&v[1]);
}  // <- here
