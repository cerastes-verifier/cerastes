// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 7 (two of the three pairings no fixture covers; the third, a reads site against an owns in a nested pairing, is range_struct_nested_cell.c): a cell the caller holds by a fraction (a reads(*q) beside owns_range(p, n), forwarded to a callee that needs the same fractions) and a forwarded structure atom of the element's own type (a struct P atom beside owns_range over struct P, forwarded). Parameters only: the forwarders hold no arrays
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  int y;
};

void leaf_reads(struct P *p, int n, int *q)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_reads(*q))
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_reads(*q))
{
  (void)p;
  (void)n;
  (void)q;
}

void fwd_fraction(struct P *p, int n, int *q)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_reads(*q))
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_reads(*q))
{
  leaf_reads(p, n, q);
}

void struct_atom(struct P *p, int n, struct P *q)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_owns(*q))
{
  (void)p;
  (void)n;
  (void)q;
}

void fwd_struct(struct P *p, int n, struct P *q)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_owns(*q))
{
  struct_atom(p, n, q);
}
