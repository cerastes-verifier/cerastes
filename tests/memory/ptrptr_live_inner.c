// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 items 3, 4: live(*pp) in requires and ensures: the callee compares the loaded pointer and passes it to a callee that needs it live
#include <cerastes.h>
#include <stddef.h>

int is_null(int *a)
    CERASTES_REQUIRES(cerastes_live(a))
    CERASTES_ENSURES(r, cerastes_live(a) && r == (a == NULL))
{
  return a == NULL;
}

int peek(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_live(*pp))
    CERASTES_ENSURES(r, cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_live(*pp) && r == (*pp == NULL))
{
  int *q = *pp;
  return is_null(q);
}
