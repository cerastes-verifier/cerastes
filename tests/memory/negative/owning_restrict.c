// expect: rejected cerastes-unsupported_annotation @ L9
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/restrict_dropped.c
// Pins: 0064 item 19 (question 326, the owner): every restrict parameter is rejected, unsupported_annotation, once per function at its head; drop the keyword to verify the function (tests/memory/restrict_dropped.c). Re-expected from verified by Flips R. It verified before as: 0040 item 3 (0037 item 2): owning is the owns conjunct restrict needs; no other contract is written
// Pre-registered from record 0064 item 19 (Part R, Flips R), before the run: one row per function with a restrict parameter, at its head line
#include <cerastes.h>
#include <stddef.h>

int get(CERASTES_OWNING int *restrict p)  // <- here
    CERASTES_ENSURES(r, p == NULL ? r == 0 : r == *p)
{
  if (p == NULL) {
    return 0;
  }
  return *p;
}
