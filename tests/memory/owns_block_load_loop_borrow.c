// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 2 ("Loops") and audit round 2 item 2 (probe l5b): a loop that lends the block through *pp to a borrower on every iteration (touch(pp), which returns *pp == old(*pp)); the user invariant *pp == keep says the pointer is unchanged, and the block is freed after the loop
// Pre-registered from the audit probe l5b_loop_borrow_same_inv (verifies today): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

void touch(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns_block(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns_block(*pp) && *pp == cerastes_old(*pp))
{
  **pp = 2;
}

void f(int **pp, int n)
    CERASTES_REQUIRES(0 <= n && n <= 10 && cerastes_owns(*pp) && cerastes_owns_block(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp))
{
  int *keep = *pp;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && *pp == keep)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    touch(pp);
    i += 1;
  }
  free(keep);
}
