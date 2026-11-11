// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 2, audit S8: the f__wf row prior ==> (g_req ==> g_ens) with guards that read the heap (*c > 0 against *c >= 1, read in two states) on a function WITH a body (S42 tried bodiless ones): the row holds, the body does nothing with *p
// Pre-registered from record 0058 item 2 (audit round 1, S8): verifies in both modes (observed on the committed Part N build: build/m4.5d2b2d/emul3/g1.c)
#include <cerastes.h>

void maybe([[maybe_unused]] int *p CERASTES_NOT_INVALIDATING, [[maybe_unused]] int *c)
    CERASTES_REQUIRES(cerastes_reads(*c) && (*c > 0 ? cerastes_owns(*p) : 1))
    CERASTES_ENSURES(cerastes_reads(*c) && (*c >= 1 ? cerastes_owns(*p) : 1))
{
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 1;
  int k = 2;
  maybe(&x, &k);
  return 0;
}
