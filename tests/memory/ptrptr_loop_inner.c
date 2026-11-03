// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 10 (question 105): the loop holds **pp with only requires(owns(*pp) && owns(**pp)): the object *pp designated on entry is an automatic place of the loop, through its method-entry snapshot -- WITHOUT a tie (S35 finding 6): the loop never writes *pp, so the automatic value rows keep it; the ensures' *pp == old(*pp) must follow
#include <cerastes.h>
#include <stddef.h>

void bump_n(int **pp, int n)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp) && 0 <= n && n <= 100 && **pp == 0)
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_owns(**pp) && **pp == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && **pp == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    **pp += 1;
    i += 1;
  }
}
