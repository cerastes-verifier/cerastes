// expect: error overflow @ L16
// mode: c23, c++20
// twin: tests/memory/pure_keep_fact_cell.c
// Pins: 0062 item 8 fact (iii) for a cell atom, the owns alias twin (S46 (c) (4) in cell form): the application is over an owns cell that aliases a guarded reads cell: the keep fact holds under both guards only, so the post value is not tied to the pre value and the added constant may overflow
// Pre-registered from record 0062 (Part C), before the code: overflow. the tag is a question-160 candidate (as pure_keep_fact_owns_alias.c)
#include <cerastes.h>

CERASTES_PURE int get(const int *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
{
  return *p;
}

long long keep_cell_alias(int *a, int *b, int c)
    CERASTES_REQUIRES(b == a && (c == 0 || cerastes_reads(*a)) && cerastes_owns(*b) && get(b) < 100)
    CERASTES_ENSURES(r, (c == 0 || cerastes_reads(*a)) && cerastes_owns(*b) && r == get(b) + 9223372036854775000LL);

// a bodiless declaration is read only when a verified function calls it (measured on the frozen base): this caller makes the contract above part of the run
long long keep_cell_alias_caller(int *a, int *b, int c)
    CERASTES_REQUIRES(b == a && (c == 0 || cerastes_reads(*a)) && cerastes_owns(*b) && get(b) < 100)
    CERASTES_ENSURES(r, (c == 0 || cerastes_reads(*a)) && cerastes_owns(*b))
{
  return keep_cell_alias(a, b, c);
}
