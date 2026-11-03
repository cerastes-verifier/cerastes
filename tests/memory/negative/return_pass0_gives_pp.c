// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/return_pass0_keeps.c
// Pins: 0045 item 6 (pass 0, audit round 2 S3): the callee takes *pp, so the return cannot give back owns(*pp) or the **pp read through it: permission at the return, 'returns without owning `*pp`'
#include <cerastes.h>
#include <stddef.h>

void keep(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp));

void f(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns(**pp))
{
  keep(pp);
}  // <- here
