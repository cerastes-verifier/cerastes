// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 7 and 8: a function that forwards two of its own ranges over structures (the slot row verifies where no leaf receiver of a first slot is in view: the block-and-interval row would be a false permission, S48 (c2)) and one that forwards (p, n); its caller passes disjoint halves [0, 2) and [2, 3) of one local array
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  int y;
};

void touch(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  (void)p;
  (void)n;
}

void inner(struct P *p, int n, struct P *q, int m)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_owns_range(q, m))
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_owns_range(q, m))
{
  (void)p;
  (void)n;
  (void)q;
  (void)m;
}

void outer(struct P *p, int n, struct P *q, int m)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_owns_range(q, m))
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_owns_range(q, m))
{
  inner(p, n, q, m);
}

void forward_one(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n < 100)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  touch(p, n);
}

int caller(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  outer(&v[0], 2, &v[2], 1);
  return 0;
}
