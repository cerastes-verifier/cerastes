// expect: error permission @ L29
// expect: error permission @ L35
// expect: error permission @ L41
// mode: c23, c++20
// twin: tests/memory/range_struct_forall_beside.c
// Pins: 0064 items 7, 9 and 11 (Part C2 audit round, the il-analyst's finding 3; probes z08only, m_z08only, g_z08): zero_at reads p[k] for the binder k, and the contract holds nothing that owns a's cells but the structure site (a range over structures, an owning structure with a member array of structures, the global array of structures): a read under a binder is never framed by a structure site, so it is a permission error at the forall's clause, once per function, with no unmapped Silicon error; the bodies are empty, as the probes were: a use of `a` after the failed row would be a lifetime error of its own (the twin gives each an owns_range(a, m) of its own)
// Pre-registered from il-analyst-report-9 item 2 (z08only, m_z08only, g_z08: one permission error each, "in zero_at: the contract reads p[k] without owning it"), before any run: permission at the requires line of each function
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

void in_range(struct P *p, int n, int *a, int m)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n > 1 && m > 0 && cerastes_forall(zero_at, a, m))
    CERASTES_ENSURES(cerastes_reads_range(p, n))
{
}

void in_member(struct B *b, int *a, int m)
    CERASTES_REQUIRES(cerastes_owns(*b) && m > 0 && cerastes_forall(zero_at, a, m))
    CERASTES_ENSURES(cerastes_owns(*b))
{
}

void in_global(int *a, int m)
    CERASTES_REQUIRES(cerastes_reads(g) && m > 0 && cerastes_forall(zero_at, a, m))
    CERASTES_ENSURES(cerastes_reads(g))
{
}
