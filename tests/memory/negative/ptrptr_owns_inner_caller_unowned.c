// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/ptrptr_owns_inner.c
// Pins: 0045 item 6: the caller owns *pp but not **pp: the call's row for owns(**pp) fails (permission at the call)
#include <cerastes.h>
#include <stddef.h>

void set(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_owns(**pp));

void use(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp))
{
  set(pp);  // <- here
}
