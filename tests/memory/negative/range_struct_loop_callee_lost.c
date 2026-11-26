// expect: error invariant-entry @ L62
// mode: c23, c++20
// twin: tests/memory/range_struct_loop.c
// Pins: 0064 item 10 (trap 3: a callee's writes through a range over structures are recorded per leaf field): scribble writes p[0].x = 8 and states nothing about it; keeper had set p[0].x = 7 before the call, and the loop's invariant keeps it: the value is not known, invariant-entry at the clause (once). If the callee's write to the leaf x were not recorded, the stale 7 would pass
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: invariant-entry at the invariant clause [CORRECTED once under question 160 (first run, j2a): `(void)first;` replaced by `p[0].y = first;` (EDG C++ warns #550 otherwise), as in range_struct_loop.c]
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
  CERASTES_INVARIANT(0 <= i && i <= n && p[0].x == first)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p[i].y = 0;
    i = i + 1;
  }
}

void scribble(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 < n)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  (void)n;
  p[0].x = 8;
}

void keeper(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 < n)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  p[0].x = 7;
  scribble(p, n);
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p[0].x == 7)  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p[i].y = 0;
    i = i + 1;
  }
}
