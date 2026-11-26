// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 7: element reads p[0].x in a requires, in an ensures (also under old) and in a function's decreases, each framed by the range atom (its element slot lies in the range); the programs of the retired staging pin range_reject_struct_elem_reads.c (f_c03, f_c04, f_c05 read p[0].x under owns(*p), a structure atom on the slot, which frames the same read)
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  long y;
};

int first_x(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0 && p[0].x >= 0)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r >= 0 && r == p[0].x)
{
  (void)n;
  return p[0].x;
}

void bump_y(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[0].x == cerastes_old(p[0].x) && p[0].y == 7)
{
  (void)n;
  p[0].y = 7;
}

int dec(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0 && p[0].x >= 0)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == n)
    CERASTES_DECREASES(p[0].x)
{
  (void)p;
  return n;
}

int atom_requires(struct P *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p[0].x > 0)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  return p->x;
}

void atom_old(struct P *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p[0].x == cerastes_old(p[0].x))
{
  (void)p;
}

int atom_decreases(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && p[0].x >= 0)
    CERASTES_ENSURES(r, cerastes_owns(*p))
    CERASTES_DECREASES(p[0].x)
{
  (void)p;
  return n;
}
