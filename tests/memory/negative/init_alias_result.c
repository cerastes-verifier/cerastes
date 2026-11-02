// expect: error permission @ L10
// mode: c23, c++20
// twin: tests/memory/init_alias_unmarked.c
// Pins: 0042 item 6 (audit S4): ensures(r == p) ensures(*r == 1) on a ref_to_uninit parameter: the flag is not promised on return, so the read through the result alias is not framed
#include <cerastes.h>

int *pass(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == p)
    CERASTES_ENSURES(r, *r == *r)  // <- here
{
  *p = 1;
  return p;
}
