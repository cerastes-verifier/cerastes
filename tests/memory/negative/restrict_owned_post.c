// expect: error postcondition @ L13
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/restrict_owned.c
// Pins: 0037 item 10: the contract promises 1 for NULL, the body returns 0
#include <cerastes.h>
#include <stddef.h>

int get(int *restrict p)
    CERASTES_REQUIRES(p == NULL || cerastes_owns(*p))
    CERASTES_ENSURES(r, (p == NULL || cerastes_owns(*p)) && r == (p == NULL ? 1 : *p))
{
  if (p == NULL) {
    return 0;
  }
  return *p;
}

void copy_cell(int *restrict dst, int *restrict src)
    CERASTES_REQUIRES(cerastes_owns(*dst) && cerastes_owns(*src))
    CERASTES_ENSURES(cerastes_owns(*dst) && cerastes_owns(*src) && *dst == *src && *src == cerastes_old(*src))
{
  *dst = *src;
}
