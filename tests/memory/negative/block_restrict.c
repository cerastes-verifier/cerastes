// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/restrict_dropped.c
// Pins: 0064 item 19 (question 326, the owner): every restrict parameter is rejected, unsupported_annotation, once per function at its head; drop the keyword to verify the function (tests/memory/restrict_dropped.c). Re-expected from verified by Flips R. It verified before as: 0052 item 6d (the former pin reject_owns_block_restrict.c): restrict beside a parameter whose only atom is a block site: restrict_backed counts the block site, so the contract owns what restrict promises
// Pre-registered from record 0064 item 19 (Part R, Flips R), before the run: one row per function with a restrict parameter, at its head line
#include <cerastes.h>
#include <stdlib.h>

int get(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns_block(p))
    CERASTES_ENSURES(r, cerastes_owns_block(p) && r == *p)
{
  return *p;
}

int use_get(void)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 7;
  int v = get(p);
  free(p);
  return v;
}
