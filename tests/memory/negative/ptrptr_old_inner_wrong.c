// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/memory/ptrptr_old_inner.c
// Pins: 0045 item 3: old(**pp) + 2 against the body's + 1: postcondition at the closing brace
#include <cerastes.h>
#include <stddef.h>

void inc2(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp) && **pp < 100)
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_owns(**pp) && **pp == cerastes_old(**pp) + 2)
{
  **pp += 1;
}  // <- here
