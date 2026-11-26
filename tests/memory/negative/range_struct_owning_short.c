// expect: error permission @ L25
// mode: c23, c++20
// twin: tests/memory/range_struct_owning.c
// Pins: 0060 item 13 over structures: the call says four elements, owning(n) takes owns_range(p, 4) and the caller holds three: the shape row fails at the call (permission), once, and the path ends (as range_owning_param_short.c)
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: permission at the call
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  int y;
};

void zero_first(int n, struct P *p CERASTES_OWNING_N(n))
{
  if (p != NULL && n > 0) {
    p[0].x = 0;
  }
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  zero_first(4, v);  // <- here
  zero_first(0, NULL);
  return 0;
}
