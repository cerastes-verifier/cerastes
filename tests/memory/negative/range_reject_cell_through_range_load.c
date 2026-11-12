// expect: rejected cerastes-unsupported_annotation @ L11
// expect: rejected cerastes-unsupported_annotation @ L12
// mode: c23, c++20
// twin: tests/memory/range_get_set.c
// Pins: 0060 item 1 (M4.8; audit round 2, S2): a cell atom reached through a load of the range parameter (owns(**pp) beside owns_range(pp, n)) is a range beside a cell of the same parameter: rejected at the requires clause and again at the ensures clause (one in each: each clause that holds the range atom is reported, question 160); the program has no cell atom in the ensures
// Pre-registered from record 0060 items 1 and 3 (audit round 2), before the code
#include <cerastes.h>
#include <stddef.h>

void f(int **pp, int n)
    CERASTES_REQUIRES(cerastes_owns_range(pp, n) && n > 0 && cerastes_owns(**pp))
    CERASTES_ENSURES(cerastes_owns_range(pp, n))
{
  (void)pp;
  (void)n;
}
