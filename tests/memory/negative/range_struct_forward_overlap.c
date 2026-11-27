// expect: error permission @ L50
// mode: c23, c++20
// twin: tests/memory/range_struct_forward.c
// Pins: 0064 item 7 (the forwarding trap): the caller passes (&v[0], 3) and (&v[2], 1) to the forwarder of two ranges: the two ranges share v[2], the slot row fails at the call (permission, once)
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: permission at the call
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
  outer(&v[0], 3, &v[2], 1);  // <- here
  return 0;
}
