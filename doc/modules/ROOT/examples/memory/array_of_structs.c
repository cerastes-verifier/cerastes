// expect: verified
// expect: native-ok
// mode: c23, c++20
// An array of structures through a range contract: a callee that owns the range writes two elements, and a
// callee that reads it sums one member of them; the caller holds a local array of points.
// tag::source[]
#include <cerastes.h>

struct point {
  int x;
  int y;
};

void fill(struct point *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n >= 2)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[0].x == 1 && p[1].x == 3)
{
  (void)n;
  p[0].x = 1;
  p[0].y = 2;
  p[1].x = 3;
  p[1].y = 4;
}

long long sum_x(struct point *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n >= 2)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && r == (long long)p[0].x + p[1].x)
{
  (void)n;
  return (long long)p[0].x + p[1].x;
}

int demo(void)
    CERASTES_ENSURES(r, r == 4)
{
  struct point v[3] = {{0, 0}, {0, 0}, {0, 0}};
  fill(v, 3);
  return (int)sum_x(v, 3);
}
// end::source[]
