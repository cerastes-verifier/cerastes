// expect: error postcondition @ L12
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/owning_restrict.c
// Pins: 0040 item 3: the contract promises 1 for NULL, the body returns 0
#include <cerastes.h>
#include <stddef.h>

int get(CERASTES_OWNING int *restrict p)
    CERASTES_ENSURES(r, p == NULL ? r == 1 : r == *p)
{
  if (p == NULL) {
    return 0;
  }
  return *p;
}
