// expect: error permission @ L47
// mode: c23, c++20
// twin: tests/memory/range_struct_alias.c
// Pins: 0064 item 7 (alias trap 1): f(v, 3, &v[1].x) against owns_range(p, n) && owns(*q): the cell is the x of element 1, which the range holds: the call's pairing row fails (permission, once, at the call), not Silicon's own precondition
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: permission at the call
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  int y;
};

void cell_f(struct P *p, int n, int *q)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_owns(*q) && *q == 1)
{
  (void)p;
  (void)n;
  *q = 1;
}

void struct_f(struct P *p, int n, struct P *q)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_owns(*q) && q->x == 1)
{
  (void)p;
  (void)n;
  q->x = 1;
}

void ranges_f(struct P *p, int n, struct P *q, int m)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_owns_range(q, m) && n > 0 && m > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_owns_range(q, m) && p[0].x == 1 && q[0].x == 2)
{
  (void)n;
  (void)m;
  p[0].x = 1;
  q[0].x = 2;
}

int by_cell(void)
    CERASTES_ENSURES(r, r == 1)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  int z = 0;
  cell_f(v, 3, &v[1].x);  // <- here
  return z;
}

int by_struct(void)
    CERASTES_ENSURES(r, r == 1)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  struct P w = {0, 0};
  struct_f(v, 3, &w);
  return w.x;
}

int by_ranges(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  ranges_f(&v[0], 2, &v[2], 1);
  return v[0].x + v[2].x;
}
