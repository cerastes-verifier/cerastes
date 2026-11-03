// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/memory/ptrptr_owns_inner.c
// Pins: 0045 item 6: the ensures says 6, the body writes 5: postcondition at the closing brace
#include <cerastes.h>
#include <stddef.h>

void set(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_owns(**pp) && **pp == 6)
{
  **pp = 5;
}  // <- here
