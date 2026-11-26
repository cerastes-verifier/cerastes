// expect: verified
// expect: native-ok
// mode: c23, c++20
// Points in an array: a loop that clears the y of every point, and one that adds up the x of every point,
// each through a range contract on its pointer parameter.
// tag::source[]
#include <cerastes.h>

struct point {
  int x;
  int y;
};

void clear_y(struct point *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 <= n)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p[i].y = 0;
    i = i + 1;
  }
}

long long sum_x(struct point *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && 0 <= n)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && -2147483648LL * n <= r && r <= 2147483647LL * n)
{
  long long s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && -2147483648LL * i <= s && s <= 2147483647LL * i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s = s + p[i].x;
    i = i + 1;
  }
  return s;
}

int demo(void)
    CERASTES_ENSURES(r, r == 1)
{
  struct point v[3] = {{1, 2}, {3, 4}, {5, 6}};
  clear_y(v, 3);
  long long s = sum_x(v, 3);
  return s <= 6442450941LL;
}
// end::source[]
