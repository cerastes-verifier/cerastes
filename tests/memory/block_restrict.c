// expect: verified
// expect: native-ok
// mode: c23
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// Pins: 0052 item 6d (the former pin reject_owns_block_restrict.c): restrict beside a parameter whose only atom is a block site: restrict_backed counts the block site, so the contract owns what restrict promises
// Pre-registered from record 0052 item 6d: verifies
#include <cerastes.h>
#include <stdlib.h>

int get(int *restrict p)
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
