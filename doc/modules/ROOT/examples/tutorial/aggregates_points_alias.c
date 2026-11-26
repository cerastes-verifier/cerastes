// expect: error permission @ L31
// mode: c23, c++20
// The alias trap: the array of points and the address of one of its members are handed to a function that
// needs to own both.
// Derived from aggregates_points.c.
// tag::source[]
#include <cerastes.h>

struct point {
  int x;
  int y;
};

void clear_y(struct point *p, int n, int *marker)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 <= n && cerastes_owns(*marker))
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_owns(*marker))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p[i].y = 0;
    i = i + 1;
  }
  *marker = 1;
}

int demo(void)
{
  struct point v[3] = {{1, 2}, {3, 4}, {5, 6}};
  clear_y(v, 3, &v[0].y); // <- error
  return v[0].x;
}
// end::source[]
