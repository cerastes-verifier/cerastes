// expect: error permission @ L44
// expect: error permission @ L44
// mode: c23, c++20
// twin: tests/memory/range_struct_forward.c
// Pins: 0064 item 8 (as range_call_fwd_one_more.c): forwarding (p, n + 1) of the function's own range of structures: the shape row fails at the call (permission, once)
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: permission at the call [CORRECTED once under question 160 (first run, j2a): the call draws TWO permission errors at the same line (the shape row, "calls `touch`, which needs to own `p[0..n + 1)`", and the marked row of the leaf, the same with "(the member `x` of every element)"), both for the one reason this twin is about; the header now registers both]
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
  touch(p, n + 1);  // <- here
}

int caller(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  outer(&v[0], 2, &v[2], 1);
  return 0;
}
