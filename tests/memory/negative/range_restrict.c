// expect: rejected cerastes-unsupported_annotation @ L12
// expect: rejected cerastes-unsupported_annotation @ L20
// expect: rejected cerastes-unsupported_annotation @ L29
// mode: c23
// c-only: restrict does not exist in C++20 (decision 0037 item 2)
// twin: tests/memory/restrict_dropped.c
// Pins: 0064 item 19 (question 326, the owner): every restrict parameter is rejected, unsupported_annotation, once per function at its head; drop the keyword to verify the function (tests/memory/restrict_dropped.c). Re-expected from verified by Flips R. It verified before as: 0060 item 14 (0037 item 2): restrict parameters backed by owns_range, unguarded and after p == NULL ||, and two restrict parameters of one pointee type each backed by a range of its own. C only: restrict does not exist in C++20 (as tests/memory/block_restrict.c)
// Pre-registered from record 0064 item 19 (Part R, Flips R), before the run: one row per function with a restrict parameter, at its head line
#include <cerastes.h>
#include <stddef.h>

void set0(int *restrict p, int n)  // <- here
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[0] == 0)
{
  (void)n;
  p[0] = 0;
}

void set0_guarded(int *restrict p, int n)  // <- here
    CERASTES_REQUIRES(p == NULL || cerastes_owns_range(p, n))
    CERASTES_ENSURES(p == NULL || cerastes_owns_range(p, n))
{
  if (p != NULL && n > 0) {
    p[0] = 0;
  }
}

void two(int *restrict dst, int *restrict src, int n)  // <- here
    CERASTES_REQUIRES(cerastes_owns_range(dst, n) && cerastes_owns_range(src, n))
    CERASTES_ENSURES(cerastes_owns_range(dst, n) && cerastes_owns_range(src, n))
{
  (void)dst;
  (void)src;
  (void)n;
}
