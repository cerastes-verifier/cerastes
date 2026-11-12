// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 8 (audit round 3, S1): a range beside a global cell (owns_range with owns(g), reads_range with owns(g), owns_range with reads(g)) and beside a cell reached through a load (owns(**pp) with owns_range(q, n)): the caller passes a separate local array (and the ties *pp == old(*pp) of 0045 item 10 keep **pp's place across the calls), so the separation rows against the global and loaded cells hold
// Pre-registered from record 0060 item 8 (audit round 3), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

int g;

void f1(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_owns(g))
{
  (void)p;
  (void)n;
}

void f2(int **pp, int *q, int n)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp) && cerastes_owns_range(q, n))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_owns(**pp) && cerastes_owns_range(q, n))
{
  (void)pp;
  (void)q;
  (void)n;
}

void f3(const int *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && cerastes_owns(g))
    CERASTES_ENSURES(cerastes_reads_range(p, n) && cerastes_owns(g))
{
  (void)p;
  (void)n;
}

void f4(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_reads(g))
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_reads(g))
{
  (void)p;
  (void)n;
}

void ok1(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  int a[2] = {1, 2};
  f1(a, 2);
}

void ok2(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_owns(**pp))
{
  int a[2] = {1, 2};
  f2(pp, a, 2);
}

void ok3(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  int a[2] = {1, 2};
  f3(a, 2);
}

void ok4(void)
    CERASTES_REQUIRES(cerastes_reads(g))
    CERASTES_ENSURES(cerastes_reads(g))
{
  int a[2] = {1, 2};
  f4(a, 2);
}
