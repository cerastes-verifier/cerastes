// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 9 and 11 (Part C2, audit round C1's finding 1 for a global array of structures): a call whose requires owns g beside the use of an unrelated owned global (set_x over x) is accepted, both writes are known afterwards; a call that owns g beside a read of x, and a call that reads g beside a read of g[1].x (two reads), are accepted too. negative/global_array_struct_seq_unsequenced.c moves the read into the array the callee owns
// Pre-registered from record 0064 items 9 and 11 and the audit's finding 1, before any run: verifies
#include <cerastes.h>

struct P {
  int x;
  int y;
};

static struct P g[3] = {{1, 2}, {3, 4}};
static int x = 9;

int wr(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(r, cerastes_owns(g) && g[1].x == 7 && r == 0)
{
  g[1].x = 7;
  return 0;
}

int rd(void)
    CERASTES_REQUIRES(cerastes_reads(g))
    CERASTES_ENSURES(r, cerastes_reads(g) && r == g[1].x)
{
  return g[1].x;
}

int set_x(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q))
    CERASTES_ENSURES(r, cerastes_owns(*q) && *q == 5 && r == 0)
{
  *q = 5;
  return 0;
}

int disjoint(void)
    CERASTES_REQUIRES(cerastes_owns(g) && cerastes_owns(x))
    CERASTES_ENSURES(r, cerastes_owns(g) && cerastes_owns(x) && r == 12)
{
  int s = wr() + set_x(&x);
  return s + g[1].x + x;
}

int beside_other(void)
    CERASTES_REQUIRES(cerastes_owns(g) && cerastes_owns(x) && x == 9)
    CERASTES_ENSURES(r, cerastes_owns(g) && cerastes_owns(x) && r == 9)
{
  return wr() + x;
}

int reads_beside(void)
    CERASTES_REQUIRES(cerastes_reads(g) && g[1].x == 3)
    CERASTES_ENSURES(r, cerastes_reads(g) && r == 6)
{
  return rd() + g[1].x;
}
