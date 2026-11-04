// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 11 (question 109): a ?: arm under a heap condition: *pp != NULL ? owns(**pp) : 1
#include <cerastes.h>
#include <stddef.h>

void h(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && (*pp != NULL ? cerastes_owns(**pp) : 1))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && (*pp != NULL ? cerastes_owns(**pp) : 1))
{
  if (*pp != NULL) {
    **pp = 2;
  }
}
