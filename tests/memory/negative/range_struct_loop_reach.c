// expect: error invariant-preserved @ L39
// mode: c23, c++20
// twin: tests/memory/range_struct_loop.c
// Pins: 0064 item 10 (reach): fill_y writes the int member x while the invariant keeps p[0].x == first: a store to a leaf x of the range reaches every place of the field, so the value is not kept: invariant-preserved at the clause (once; the entry and the ensures hold)
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: invariant-preserved at the invariant clause [CORRECTED once under question 160 (first run, j2a): `(void)first;` replaced by `p[0].y = first;` (EDG C++ warns #550 otherwise), as in range_struct_loop.c]
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  long y;
};

int scan(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 < n)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && 0 <= r && r <= n)
{
  int first = p[0].x;
  int c = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && 0 <= c && c <= i && p[0].x == first)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (p[i].x > first) {
      c = c + 1;
    }
    i = i + 1;
  }
  return c;
}

void fill_y(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 < n)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[0].x == cerastes_old(p[0].x))
{
  int first = p[0].x;
  p[0].y = first;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p[0].x == first)  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p[i].x = 0;
    i = i + 1;
  }
}

void scribble(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 < n)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[0].x == 7)
{
  (void)n;
  p[0].x = 7;
}

void keeper(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 < n)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  p[0].x = 7;
  scribble(p, n);
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p[0].x == 7)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p[i].y = 0;
    i = i + 1;
  }
}
