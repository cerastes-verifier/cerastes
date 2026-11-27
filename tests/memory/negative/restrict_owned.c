// expect: rejected cerastes-unsupported_annotation @ L10
// expect: rejected cerastes-unsupported_annotation @ L20
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/restrict_dropped.c
// Pins: 0064 item 19 (question 326, the owner): every restrict parameter is rejected, unsupported_annotation, once per function at its head; drop the keyword to verify the function (tests/memory/restrict_dropped.c). Re-expected from verified by Flips R. It verified before as: 0037 item 2: restrict on a parameter whose requires owns *p (guarded or not) and whose ensures returns it
// Pre-registered from record 0064 item 19 (Part R, Flips R), before the run: one row per function with a restrict parameter, at its head line
#include <cerastes.h>
#include <stddef.h>

int get(int *restrict p)  // <- here
    CERASTES_REQUIRES(p == NULL || cerastes_owns(*p))
    CERASTES_ENSURES(r, (p == NULL || cerastes_owns(*p)) && r == (p == NULL ? 0 : *p))
{
  if (p == NULL) {
    return 0;
  }
  return *p;
}

void copy_cell(int *restrict dst, int *restrict src)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*dst) && cerastes_owns(*src))
    CERASTES_ENSURES(cerastes_owns(*dst) && cerastes_owns(*src) && *dst == *src && *src == cerastes_old(*src))
{
  *dst = *src;
}
