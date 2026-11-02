// expect: verified
// expect: native-ok
// mode: c23, c++20
// twin-exempt: a bodyless declaration is a trusted contract, nothing in it can fail; its must-fail forms are the rejected pins reject_marker_uncalled_*
// Pins: 0042 item 4 (audit round 2 S4): a correct marked declaration that nothing calls is checked and emitted as a trusted method; its rejected shapes are the reject_marker_uncalled_* pins
#include <cerastes.h>

void fill(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p));

void keep(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p));

int main(void)
{
  return 0;
}
