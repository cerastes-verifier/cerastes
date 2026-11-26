// expect: error permission @ L30
// expect: error permission @ L41
// expect: error permission @ L51
// mode: c23, c++20
// twin: tests/memory/range_struct_pure_beside.c
// Pins: 0064 items 7, 9 and 11 (Part C2 audit round, the il-analyst's finding 2; probes g02, m_g02, g_g02): first(a, m) reads a[0] under reads_range(a, m), and the contract holds nothing that owns a's cells but the structure site (a range over structures, an owning structure with a member array of structures, the global array of structures): a structure site never frames a scalar application, so the application is a permission error at the requires, once per function, with no internal error and no unmapped Silicon error (the twin gives each a reads_range(a, m) of its own)
// Pre-registered from il-analyst-report-9 item 1 (g02, m_g02, g_g02: one permission error each, rc 1), before any run: permission at the requires line of each function
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
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && m > 0 && n > 0 && first(a, m) == 7)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && r == 7)
{
  (void)p;
  (void)n;
  (void)a;
  (void)m;
  return 7;
}

int in_member(struct B *b, int *a, int m)
    CERASTES_REQUIRES(cerastes_owns(*b) && m > 0 && first(a, m) == 7)
    CERASTES_ENSURES(r, cerastes_owns(*b) && r == 7)
{
  (void)b;
  (void)a;
  (void)m;
  return 7;
}

int in_global(int *a, int m)
    CERASTES_REQUIRES(cerastes_reads(g) && m > 0 && first(a, m) == 7)
    CERASTES_ENSURES(r, cerastes_reads(g) && r == 7)
{
  (void)a;
  (void)m;
  return 7;
}
