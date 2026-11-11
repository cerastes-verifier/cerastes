// expect: error permission @ L10
// mode: c23, c++20
// twin: tests/memory/not_invalidating_guard_heap_equivalent.c
// Pins: 0058 item 2, audit S8: the same bodied function with the ensures guard *c > 5: on the path 0 < *c <= 5 the contract takes *p without returning it, which the marker forbids: the new f__wf row fails (permission at the ensures clause, as not_invalidating_guard_differs.c); the caller passes k = 7, which returns the cell, so only the row fails
// Pre-registered from record 0058 item 2 (audit round 1, S8): one permission error at the ensures clause (observed on the committed Part N build: build/m4.5d2b2d/emul3/g2.c, with the caller of g2 changed to k = 7)
#include <cerastes.h>

void maybe([[maybe_unused]] int *p CERASTES_NOT_INVALIDATING, [[maybe_unused]] int *c)
    CERASTES_REQUIRES(cerastes_reads(*c) && (*c > 0 ? cerastes_owns(*p) : 1))
    CERASTES_ENSURES(cerastes_reads(*c) && (*c > 5 ? cerastes_owns(*p) : 1))  // <- here
{
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 1;
  int k = 7;
  maybe(&x, &k);
  return 0;
}
