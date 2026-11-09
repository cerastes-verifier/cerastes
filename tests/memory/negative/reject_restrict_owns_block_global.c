// expect: rejected cerastes-unsupported_annotation @ L13
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/restrict_owned.c
// Pins: 0054 item 2 ("restrict": a site rooted at a global counts in restrict_hazard like an atom on that global) and audit round 2 item 2 (probe o4): a restrict parameter beside a global whose block a site names (a pointer loaded from g is not based on q): rejected at the function head
// Pre-registered from the audit probe o4_restrict_global as observed (c23): rejected unsupported_annotation at the function head
#include <cerastes.h>
#include <stdlib.h>

static int *g = NULL;

int *peek(void) { return g; }

void f(int *restrict q)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*q) && cerastes_reads(g) && (g == NULL || cerastes_owns_block(g)))
    CERASTES_ENSURES(cerastes_owns(*q) && cerastes_reads(g) && (g == NULL || cerastes_owns_block(g)) && g == cerastes_old(g))
{
  *q = 1;
}
