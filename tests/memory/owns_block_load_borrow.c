// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 2 (retires reject_owns_block_load.c's shape): owns_block(*pp) in both lists BORROWS the block through *pp and returns it whole, the contract read **pp == n is framed by the site, and the caller reads it and frees the block
// Pre-registered from record 0054 item 2 ("borrowed", S39 a'): verifies in both modes (s39_a_consume_borrow.vpr)
#include <cerastes.h>
#include <stdlib.h>

void set(int **pp, int n)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns_block(*pp) && 0 <= n && n < 100)
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns_block(*pp) && **pp == n)
{
  **pp = n;
}

int use_set(void)
    CERASTES_ENSURES(r, r == 3 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 1;
  set(&p, 3);
  int v = *p;
  free(p);
  return v;
}
