// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 7, 9 and 11 (Part C2 audit round, the il-analyst's finding 2: a scalar `pure` application beside a structure site is framed by a scalar range of its own, never by the structure site): the control of negative/range_struct_pure_beside_unframed.c. first(a, m) reads a[0] under reads_range(a, m); the same contract holds a range over structures (in_range), an owning structure with a member array of structures (in_member), or the global array of structures (in_global), and its own reads_range(a, m) frames the application
// Pre-registered from il-analyst-report-9 item 1 (m_g02ok verified on the lane; the range and the global forms are the VIR cases `e1e2c2_apply_beside_{range,global}_framed_by_scalar_range`), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  int y;
};

struct B {
  struct P v[3];
  int k;
};

struct P g[3];

CERASTES_PURE int first(const int *a, [[maybe_unused]] int m)
    CERASTES_REQUIRES(cerastes_reads_range(a, m) && m > 0)
{
  return a[0];
}

int in_range(struct P *p, int n, int *a, int m)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && cerastes_reads_range(a, m) && m > 0 && n > 0 && first(a, m) == 7)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && cerastes_reads_range(a, m) && r == 7)
{
  (void)p;
  (void)n;
  (void)m;
  return a[0];
}

int in_member(struct B *b, int *a, int m)
    CERASTES_REQUIRES(cerastes_owns(*b) && cerastes_reads_range(a, m) && m > 0 && first(a, m) == 7)
    CERASTES_ENSURES(r, cerastes_owns(*b) && cerastes_reads_range(a, m) && r == 7)
{
  (void)b;
  (void)m;
  return a[0];
}

int in_global(int *a, int m)
    CERASTES_REQUIRES(cerastes_reads(g) && cerastes_reads_range(a, m) && m > 0 && first(a, m) == 7)
    CERASTES_ENSURES(r, cerastes_reads(g) && cerastes_reads_range(a, m) && r == 7)
{
  (void)m;
  return a[0];
}
