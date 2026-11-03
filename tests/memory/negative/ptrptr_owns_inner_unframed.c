// expect: error permission @ L9
// mode: c23, c++20
// twin: tests/memory/ptrptr_owns_inner.c
// Pins: 0045 items 3, 4: owns(**pp) before owns(*pp): the contract reads *pp without owning it (permission at the clause)
#include <cerastes.h>
#include <stddef.h>

void set(int **pp)
    CERASTES_REQUIRES(cerastes_owns(**pp) && cerastes_owns(*pp))  // <- here
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns(**pp))
{
  **pp = 5;
}
