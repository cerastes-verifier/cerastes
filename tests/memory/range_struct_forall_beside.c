// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 7, 9 and 11 (Part C2 audit round, the il-analyst's finding 3: a `cerastes_forall` over a scalar range beside a structure site is framed by the scalar range, never by the structure site): the control of negative/range_struct_forall_beside_unframed.c. zero_at reads p[k] under the binder; the same contract holds a range over structures (in_range, probe z08ok), an owning structure with a member array of structures (in_member, m_z08) or the global array of structures (in_global, g_z08ok), and the owns_range(a, m) frames the forall and its write a[0] = 0 keeps it
// Pre-registered from il-analyst-report-9 item 2 (z08ok, m_z08 and g_z08ok verified on the lane), before any run: verifies
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

CERASTES_PURE bool zero_at(int k, const int *p, int m)
{
  return !(0 <= k && k < m) || p[k] == 0;
}

int in_range(struct P *p, int n, int *a, int m)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && cerastes_owns_range(a, m) && n > 1 && m > 0 && cerastes_forall(zero_at, a, m) && p[1].x == 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && cerastes_owns_range(a, m) && cerastes_forall(zero_at, a, m) && p[1].x == 0)
{
  (void)p;
  (void)n;
  (void)m;
  a[0] = 0;
  return 0;
}

int in_member(struct B *b, int *a, int m)
    CERASTES_REQUIRES(cerastes_owns(*b) && cerastes_owns_range(a, m) && m > 0 && cerastes_forall(zero_at, a, m))
    CERASTES_ENSURES(r, cerastes_owns(*b) && cerastes_owns_range(a, m) && cerastes_forall(zero_at, a, m))
{
  (void)b;
  (void)m;
  a[0] = 0;
  return 0;
}

int in_global(int *a, int m)
    CERASTES_REQUIRES(cerastes_reads(g) && cerastes_owns_range(a, m) && m > 0 && cerastes_forall(zero_at, a, m))
    CERASTES_ENSURES(r, cerastes_reads(g) && cerastes_owns_range(a, m) && cerastes_forall(zero_at, a, m))
{
  (void)m;
  a[0] = 0;
  return 0;
}
