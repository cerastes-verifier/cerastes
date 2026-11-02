// expect: rejected cerastes-uninit_ref_initialized @ L16
// mode: c23, c++20
// twin: tests/memory/init_strict_ok.c
// verify-options: --strict-init
// Pins: 0042 item 8 (question 83): the caller's own marked parameter is accepted only while it is never assigned
#include <cerastes.h>
void init(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 0);

void g(int *p CERASTES_MUST_INIT, int *r CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*r))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns(*r))
{
  p = r;
  init(p);  // <- here
}
