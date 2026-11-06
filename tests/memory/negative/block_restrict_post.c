// expect: error postcondition @ L13
// mode: c23
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/block_restrict.c
// Pins: 0052 item 6d: the contract promises the value of *p and the body returns 0
// Pre-registered from record 0052 item 6d: postcondition at the return
#include <cerastes.h>

int get(int *restrict p)
    CERASTES_REQUIRES(cerastes_owns_block(p))
    CERASTES_ENSURES(r, cerastes_owns_block(p) && r == *p)
{
  return 0;  // <- here
}
