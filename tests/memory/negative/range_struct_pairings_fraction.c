// expect: error permission @ L27
// mode: c23, c++20
// twin: tests/memory/range_struct_pairings.c
// Pins: 0064 item 7 (a cell the caller holds by a fraction): the forwarder holds *q by a read amount and passes it to a callee that needs to own it: the amount row fails at the call (permission, once)
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: permission at the call
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  int y;
};

void leaf_reads(struct P *p, int n, int *q)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_owns(*q))
{
  (void)p;
  (void)n;
  (void)q;
}

void fwd_fraction(struct P *p, int n, int *q)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_reads(*q))
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_reads(*q))
{
  leaf_reads(p, n, q);  // <- here
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
