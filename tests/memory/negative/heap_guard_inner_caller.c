// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/heap_guard_inner.c
// Pins: 0045 item 11: the caller does not know *pp is null and does not own **pp: the guarded row fails (permission at the call)
#include <cerastes.h>
#include <stddef.h>

void h(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && (*pp == NULL || cerastes_owns(**pp)))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && (*pp == NULL || cerastes_owns(**pp)));

void use(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp))
{
  h(pp);  // <- here
}
