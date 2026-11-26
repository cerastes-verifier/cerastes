// expect: error permission @ L44
// expect: error permission @ L44
// mode: c23, c++20
// twin: tests/memory/range_struct_pairings.c
// Pins: 0064 item 7 (a forwarded structure atom of the element's own type): the forwarder passes &p[0], an element its own range holds, in place of its structure atom q: the pairing row fails at the call (permission, once)
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: permission at the call [CORRECTED once under question 160 (first run, j2a): the call draws TWO permission errors at the same line: "calls `struct_atom`, which needs to own `p[0]`, without owning it" (the forwarder holds a range, not a structure atom on that place) and the pairing row, "passes `p[0..n)` and `p[0]` ... which may be one object"; both are the reason this twin is about, the header registers both]
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
  struct_atom(p, n, &p[0]);  // <- here
}
