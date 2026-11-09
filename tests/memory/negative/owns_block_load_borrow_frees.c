// expect: error permission @ L15
// mode: c23, c++20
// twin: tests/memory/owns_block_load_borrow.c
// Pins: 0054 item 2 ("borrowed"): the borrower freed the block it promises back (no value claim in the ensures, so only the block site's return row fails)
// Pre-registered from record 0054 item 2 (S39 ap1y: borrow frees): permission at the closing brace
#include <cerastes.h>
#include <stdlib.h>

void set(int **pp, int n)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns_block(*pp) && 0 <= n && n < 100)
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns_block(*pp))
{
  **pp = n;
  free(*pp);
}  // <- here
