// expect: error postcondition @ L14
// mode: c23, c++20
// twin: tests/memory/owns_block_load_borrow.c
// Pins: 0054 item 2: the contract read **pp == n after the site is a real obligation of the callee (n may be non-zero)
// Pre-registered from record 0054 item 2: postcondition at the closing brace
#include <cerastes.h>
#include <stdlib.h>

void set(int **pp, int n)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns_block(*pp) && 0 <= n && n < 100)
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns_block(*pp) && **pp == n)
{
  **pp = 0;
}  // <- here
