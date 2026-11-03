// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/memory/ptrptr_depth3_inner.c
// Pins: 0045 item 6: ***ppp == 8 against the body's 7: postcondition at the closing brace
#include <cerastes.h>
#include <stddef.h>

void set3(int ***ppp)
    CERASTES_REQUIRES(cerastes_owns(*ppp) && cerastes_owns(**ppp) && cerastes_owns(***ppp))
    CERASTES_ENSURES(cerastes_owns(*ppp) && *ppp == cerastes_old(*ppp) && cerastes_owns(**ppp) && **ppp == cerastes_old(**ppp) && cerastes_owns(***ppp) && ***ppp == 8)
{
  ***ppp = 7;
}  // <- here
