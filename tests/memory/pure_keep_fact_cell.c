// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 item 8 fact (iii) for a CELL atom (audit round 3 R3-S1; the cell form of pure_keep_fact.c): a reads(*p) atom in the requires and its partner in the ensures: the post-state application equals the pre-state one, so the value stays below 100 and the added constant does not overflow a long long
// Pre-registered from record 0062 (Part C), before the code: verifies
#include <cerastes.h>

CERASTES_PURE int get(const int *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
{
  return *p;
}

long long keep_cell(const int *p)
    CERASTES_REQUIRES(cerastes_reads(*p) && get(p) < 100)
    CERASTES_ENSURES(r, cerastes_reads(*p) && r == get(p) + 9223372036854775000LL);

// a bodiless declaration is read only when a verified function calls it (measured on the frozen base): this caller makes the contract above part of the run
long long keep_cell_caller(const int *p)
    CERASTES_REQUIRES(cerastes_reads(*p) && get(p) < 100)
    CERASTES_ENSURES(r, cerastes_reads(*p))
{
  return keep_cell(p);
}
