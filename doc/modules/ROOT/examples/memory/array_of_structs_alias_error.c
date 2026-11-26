// expect: error permission @ L25
// mode: c23, c++20
// An element of the array is not another object: a function that owns the range and a cell cannot be given
// the range and the address of one of its members.
// tag::source[]
#include <cerastes.h>

struct point {
  int x;
  int y;
};

void tally(struct point *p, int n, int *count)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_owns(*count))
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_owns(*count))
{
  (void)p;
  (void)n;
  *count = 0;
}

int demo(void)
{
  struct point v[3] = {{1, 2}, {3, 4}, {5, 6}};
  tally(v, 3, &v[1].x); // <- error
  return v[0].x;
}
// end::source[]
