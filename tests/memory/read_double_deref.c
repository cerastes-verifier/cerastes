// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 3 (flip of reject_read_double_deref.c): a read of **pp in a contract once an atom owns it
#include <cerastes.h>
#include <stddef.h>

void f(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp) && **pp > 0)
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_owns(**pp))
{
  (void)pp;
}
