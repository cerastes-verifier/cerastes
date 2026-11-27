// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 7 and 8: reads_range(p, n) over a structure: a callee reads p[i].y and states it in its ensures (framed by the range), and a caller passes a local array of structures; the read amount comes back (the minimum over each leaf path)
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  int y;
};

int get(struct P *p, int n, int i)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && 0 <= i && i < n)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && r == p[i].y)
{
  (void)n;
  return p[i].y;
}

int use(void)
    CERASTES_ENSURES(r, r == 4)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  return get(v, 3, 1);
}
