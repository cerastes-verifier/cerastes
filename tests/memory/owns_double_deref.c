// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 3 (flip of reject_owns_double_deref.c): owns(**pp) with owns(*pp) before it
#include <cerastes.h>
#include <stddef.h>

void f(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_owns(**pp))
{
  (void)pp;
}
