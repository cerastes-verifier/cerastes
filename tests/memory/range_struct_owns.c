// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 7 and 8: owns_range(p, n) over a structure: a callee writes p[i].x and both leaves of p[0] under it, its ensures read the elements it wrote (framed by the range), and a caller passes a local array of structures, whole, and reads the element back (the call's rows per leaf path, the return rows give the range back)
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: verifies
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
  set(v, 3, 1, 9);
  return v[1].x;
}

struct S {
  int x;
};

void only_requires(struct S *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
{
  (void)p;
  (void)n;
}
