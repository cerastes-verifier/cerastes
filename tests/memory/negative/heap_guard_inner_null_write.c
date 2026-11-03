// expect: error null-dereference @ L12
// mode: c23, c++20
// twin: tests/memory/heap_guard_inner.c
// Pins: 0045 item 11: the body writes **pp although *pp may be null: null-dereference at the *
#include <cerastes.h>
#include <stddef.h>

void g(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && (*pp == NULL || cerastes_owns(**pp)))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && (*pp == NULL || cerastes_owns(**pp)))
{
  **pp = 1;  // <- here
}
