// expect: error permission @ L36
// mode: c23, c++20
// twin: tests/memory/range_struct_owns.c
// Pins: 0064 item 8: the call says four elements and the caller holds three: the shape row fails at the call (permission), as for the scalar range (range_call_too_many.c), and the path ends
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: permission at the call
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  int y;
};

void set(struct P *p, int n, int i, int v)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 <= i && i < n)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[i].x == v)
{
  (void)n;
  p[i].x = v;
}

void both(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[0].x == 1 && p[0].y == 2)
{
  (void)n;
  p[0].x = 1;
  p[0].y = 2;
}

int use(void)
    CERASTES_ENSURES(r, r == 9)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  both(v, 3);
  set(v, 4, 1, 9);  // <- here
  return v[1].x;
}
