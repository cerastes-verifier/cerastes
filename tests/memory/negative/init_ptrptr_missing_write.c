// expect: error postcondition @ L15
// mode: c23, c++20
// twin: tests/memory/init_ptrptr.c
// Pins: 0042 item 5: the pointer cell is not written on every path: postcondition at the return
#include <cerastes.h>
#include <stddef.h>

void make(int **pp CERASTES_MUST_INIT, int c)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp))
{
  if (c != 0) {
    *pp = NULL;
  }
}  // <- here
