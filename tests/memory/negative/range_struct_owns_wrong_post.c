// expect: error postcondition @ L20
// mode: c23, c++20
// twin: tests/memory/range_struct_owns.c
// Pins: 0064 item 7: set promises p[i].y == v and writes only x: the element read of the ensures is framed by the range, so the postcondition fails (once, at the closing brace); the caller is fine, since the weaker x fact is still stated
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: postcondition at the closing brace of set
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  int y;
};

void set(struct P *p, int n, int i, int v)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 <= i && i < n)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[i].x == v && p[i].y == v)
{
  (void)n;
  p[i].x = v;
}  // <- here

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
  set(v, 3, 1, 9);
  return v[1].x;
}
