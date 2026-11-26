// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 11 (audit round C1, finding 1; the sequencing rows of a call that holds a range over structures): two calls in one expression over the DISJOINT halves [0, 2) and [2, 4) of one array are accepted (the pair row holds for every leaf path) and both writes are known afterwards; a call beside a read of an element OUTSIDE the callee's range ([0, 2) against v[2].x) and a reads_range call beside a read of an element it reads are accepted too. The twins in negative/range_struct_seq_unsequenced.c move one operand into the callee's range
// Pre-registered from record 0064 item 11 and the audit's finding 1 (s02i with a right postcondition), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  int y;
};

int wr(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 1)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && p[1].x == 7 && r == 0)
{
  (void)n;
  p[1].x = 7;
  return 0;
}

int rd(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n > 1)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && r == p[1].x)
{
  (void)n;
  return p[1].x;
}

int halves(void)
    CERASTES_ENSURES(r, r == 14)
{
  struct P v[4] = {{1, 2}, {3, 4}, {5, 6}, {7, 8}};
  int s = wr(v, 2) + wr(&v[2], 2);
  return s + v[1].x + v[3].x;
}

int beside_outside(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  return wr(v, 2) + v[2].x;
}

int reads_beside(void)
    CERASTES_ENSURES(r, r == 6)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  return rd(v, 3) + v[1].x;
}
