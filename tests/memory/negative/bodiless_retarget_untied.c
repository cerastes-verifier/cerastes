// expect: error permission @ L10
// mode: c23, c++20
// twin: tests/memory/bodiless_retarget_tied.c
// Pins: 0045 item 5 (audit B1): without a body nothing proves the ensures atom live(*pp) is the object the requires took (a retarget could return live of any object): permission at the contract (the f__wf row)
#include <cerastes.h>
#include <stddef.h>

void retarget(int **pp, int *q)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_live(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_live(*pp));  // <- here

void use(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_live(*pp))
{
  retarget(pp, NULL);
}
