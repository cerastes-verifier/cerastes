// expect: rejected cerastes-unsupported_annotation @ L9
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/restrict_dropped.c
// Pins: 0054 item 2 ("restrict": a site through a load does not back restrict on its root formal) and audit round 2 item 2 (probe o5): restrict on the root of a site through a load, beside another restrict pointer: rejected at the function head [0064 item 19, question 326: every restrict parameter is now rejected by that one rule, whatever the contract; this pin's row keeps its tag and line and its condition no longer matters]
// Pre-registered from the audit probe o5_restrict_load as observed (c23): rejected unsupported_annotation at the function head
#include <cerastes.h>
#include <stdlib.h>

void f(int **restrict pp, int *restrict q)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns_block(*pp) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns_block(*pp) && cerastes_owns(*q))
{
  **pp = *q;
}
