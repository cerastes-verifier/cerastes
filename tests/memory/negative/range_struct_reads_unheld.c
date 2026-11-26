// expect: error permission @ L24
// mode: c23, c++20
// twin: tests/memory/range_struct_reads.c
// Pins: 0064 item 8: a caller that does not hold the array (a parameter with no atom) calls a function that needs reads_range(p, 3) over structures: the call's rows fail (permission, once)
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: permission at the call
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

int use(struct P *q)
{
  return get(q, 3, 1);  // <- here
}
